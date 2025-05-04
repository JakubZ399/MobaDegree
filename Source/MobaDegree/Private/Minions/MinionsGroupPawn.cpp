// 2025 Jakub Żurawik. All Rights Reserved.

#include "Minions/MinionsGroupPawn.h"

#include "AIController.h"
#include "GroomVisualizationData.h"
#include "Actor/MobaTower.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Chaos/PBDSuspensionConstraintData.h"
#include "Component/TeamComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Minions/MinionBase.h"
#include "MobaDegree/MobaDegreeCharacter.h"
#include "Perception/PawnSensingComponent.h"

AMinionsGroupPawn::AMinionsGroupPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnPointsMinion = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPointMinion"));
	SetRootComponent(SpawnPointsMinion);
	
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = 300.f;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);

#pragma region MinionsSpawnPoints
	SpawnPointMinionMelee = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPointMinionMeele"));
	SpawnPointMinionMelee->SetupAttachment(SpawnPointsMinion);

	SpawnPointMinionRanged = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPointMinionRanged"));
	SpawnPointMinionRanged->SetupAttachment(SpawnPointsMinion);

	SpawnPoint_Melee_Middle = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint_Melee_Middle"));
	SpawnPoint_Melee_Middle->SetupAttachment(SpawnPointMinionMelee);

	SpawnPoint_Melee_Right = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint_Melee_Right"));
	SpawnPoint_Melee_Right->SetupAttachment(SpawnPointMinionMelee);

	SpawnPoint_Melee_Left = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint_Melee_Left"));
	SpawnPoint_Melee_Left->SetupAttachment(SpawnPointMinionMelee);
	
	SpawnPoint_Ranged_Middle = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint_Ranged_Middle"));
	SpawnPoint_Ranged_Middle->SetupAttachment(SpawnPointMinionRanged);

	SpawnPoint_Ranged_Right = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint_Ranged_Right"));
	SpawnPoint_Ranged_Right->SetupAttachment(SpawnPointMinionRanged);
	
	SpawnPoint_Ranged_Left = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint_Ranged_Left"));
	SpawnPoint_Ranged_Left->SetupAttachment(SpawnPointMinionRanged);
#pragma endregion
}

void AMinionsGroupPawn::BeginPlay()
{
	Super::BeginPlay();

	AttackTarget = nullptr;
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(FindActorTimerHandle, this, &ThisClass::FindValidAttackTarget, ScanTime, true);
	}
}

void AMinionsGroupPawn::FindValidAttackTarget()
{
	if (AttackTarget)
	{
		if (AttackTarget->IsA(AMobaDegreeCharacter::StaticClass()))
		{
			TArray<AActor*> CurrentTargets = FindActorsInRange();
			if (!CurrentTargets.Contains(AttackTarget))
			{
				UnBindBindEnemyDestroy(AttackTarget);
				OnEnemyDestroyed(nullptr);
				AttackTarget = nullptr;
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	
	TargetsInRange = FindActorsInRange();

	if (SpawnedMinions.Num() == 0)
	{
		Destroy();
	}
	
	if (TargetsInRange.Num() > 0)
	{
		CategorizedTargets.Empty();
		
		for (AActor* TargetActor : TargetsInRange)
		{
			ETargetTypePriority Priority = ETargetTypePriority::None;
			
			if (TargetActor->IsA(AMobaTower::StaticClass()))
			{
				Priority = ETargetTypePriority::Tower;
			}
			else if (AMinionsGroupPawn* TargetGroup = Cast<AMinionsGroupPawn>(TargetActor))
			{
				Priority = ETargetTypePriority::MinionGroup;
			}
			else if (TargetActor->IsA(AMobaDegreeCharacter::StaticClass()))
			{
				Priority = ETargetTypePriority::Player;
			}

			if (Priority != ETargetTypePriority::None)
			{
				if (!CategorizedTargets.Contains(Priority))
				{
					CategorizedTargets.Add(Priority, TArray<AActor*>());
				}
				CategorizedTargets[Priority].Add(TargetActor);
			}
		}
		SelectTargetByPriority();
	}
	else if (TargetsInRange.Num() == 0)
	{
		OnEnemyDestroyed(nullptr);
	}
}

void AMinionsGroupPawn::SelectTargetByPriority()
{
	TArray<ETargetTypePriority> PriorityOrder = {
	ETargetTypePriority::Tower,
	ETargetTypePriority::MinionGroup,
	ETargetTypePriority::Player,
	};

	for (ETargetTypePriority Priority : PriorityOrder)
	{
		if (CategorizedTargets.Contains(Priority))
		{
			TArray<AActor*>& Targets = CategorizedTargets[Priority];
			if (Targets.Num() > 0)
			{
				AActor* SelectedTarget = FindClosestTarget(Targets);
				
				HandleSelectedTarget(SelectedTarget, Priority);
				return;
			}
		}
	}
}

void AMinionsGroupPawn::HandleSelectedTarget(AActor* Target, ETargetTypePriority Priority)
{
	switch (Priority)
	{
	case ETargetTypePriority::Tower:
		if (Target->IsA(AMobaTower::StaticClass()))
		{
			SetupDetectedEnemy(bDetectTower, Target);
			if (bDebugMode)
			{
				DrawDebugCapsule(GetWorld(), Target->GetActorLocation(), 65.f, 45.f, FQuat::Identity, FColor::Red, false, 1.f);
			}
		}
		break;
            
	case ETargetTypePriority::MinionGroup:
		if (Target->IsA(AMinionsGroupPawn::StaticClass()))
		{
			AMinionBase* ClosestMinion = FindClosestMinionFromGroup(Target);
			SetupDetectedEnemy(bDetectMinion, ClosestMinion);
			if (bDebugMode)
			{
				DrawDebugCapsule(GetWorld(), Target->GetActorLocation(), 65.f, 45.f, FQuat::Identity, FColor::Red, false, 1.f);
			}
		}
		break;
            
	case ETargetTypePriority::Player:
		if (Target->IsA(AMobaDegreeCharacter::StaticClass()))
		{
			if (bDebugMode)
			{
				DrawDebugCapsule(GetWorld(), Target->GetActorLocation(), 65.f, 45.f, FQuat::Identity, FColor::Red, false, 1.f);
			}
			SetupDetectedEnemy(bDetectPlayers, Target);
		}
		break;
            
	default:
		break;
	}
}

AMinionBase* AMinionsGroupPawn::FindClosestMinionFromGroup(AActor* EnemyGroup)
{
	AMinionsGroupPawn* Group = Cast<AMinionsGroupPawn>(EnemyGroup);
	
	if (!Group || Group->SpawnedMinions.Num() == 0)
		return nullptr;
    
	AMinionBase* ClosestMinion = nullptr;
	float MinDistance = FLT_MAX;
    
	for (AMinionBase* Minion : Group->SpawnedMinions)
	{
		if (Minion && !Minion->IsPendingKillPending())
		{
			float Distance = GetDistanceTo(Minion);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestMinion = Minion;
			}
		}
	}
    
	return ClosestMinion;
}

AActor* AMinionsGroupPawn::FindClosestTarget(const TArray<AActor*>& Targets)
{
	AActor* ClosestTarget = nullptr;
	float MinDistance = FLT_MAX;
    
	for (AActor* Target : Targets)
	{
		float Distance = GetDistanceTo(Target);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestTarget = Target;
		}
	}
    
	return ClosestTarget;
}

TArray<AActor*> AMinionsGroupPawn::FindActorsInRange()
{
	TArray<AActor*> FoundActors;

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ScanRadius);
	FCollisionQueryParams QueryParams;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Append(SpawnedMinions);
	ActorsToIgnore.Add(this);
	QueryParams.AddIgnoredActors(ActorsToIgnore);
	
	TArray<FOverlapResult> OverlapResults;
	bool bSuccess = GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_GameTraceChannel1, SphereShape, QueryParams);
	
	if (bSuccess)
	{
		for (FOverlapResult OverlapActor : OverlapResults)
		{
			AActor* TargetActor = OverlapActor.GetActor();
			if (TargetActor)
			{
				if (TargetActor->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()) && !TargetActor->IsA(AMinionBase::StaticClass()))
				{
					EGameTeam ActorTeam = IMobaTeamInterface::Execute_GetTeamInterface(TargetActor);

					bool bIsEnemy = (ActorTeam != Team);
					if (bIsEnemy)
					{
						FoundActors.Add(TargetActor);
						if (bDebugMode)
						{
							DrawDebugSphere(GetWorld(), GetActorLocation(), ScanRadius, 16, FColor::Red, false, ScanTime);
						}
					}
				}
			}
		}
		
		if (bDebugMode)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ScanRadius, 16, FColor::Green, false, ScanTime);
		}
		return FoundActors;
	}
	else
	{
		if (bDebugMode)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ScanRadius, 16, FColor::Yellow, false, ScanTime);
		}
	}

	return FoundActors;
}

void AMinionsGroupPawn::SetAttackTargetBlackboard(AActor* Target, FName Key)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(Key, Target);
		}
	}
}

void AMinionsGroupPawn::Initialize()
{
	if (!EnemyLaneTarget) { UE_LOG(LogTemp, Warning, TEXT("MinionsGroupSpawn: Setup EnemyLaneTarget")); return;}

	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyLaneTarget->GetActorLocation());
	SetActorRotation(NewRotation);

	SetAttackTargetBlackboard(EnemyLaneTarget, EnemyLaneTargetKey);
	
	if (bDebugRespawnMelee && MeleeMinionClass)
	{
		MinionsSpawn(MeleeMinionClass, SpawnPointMinionMelee);
	}
	if (bDebugRespawnRanged && RangedMinionClass)
	{
		MinionsSpawn(RangedMinionClass, SpawnPointMinionRanged);
	}
}

void AMinionsGroupPawn::MinionsSpawn(TSubclassOf<AMinionBase> MinionClass, USceneComponent* SpawnPointSceneComponent)
{
	if (!MinionClass || !SpawnPointSceneComponent || !HasAuthority()) return;
	
	SpawnPointSceneComponent->GetChildrenComponents(false, SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		for (USceneComponent* SpawnPoint : SpawnPoints)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			AMinionBase* SpawnedMinion = GetWorld()->SpawnActor<AMinionBase>(MinionClass, SpawnPoint->GetComponentLocation(), SpawnPoint->GetComponentRotation(), SpawnParameters);

			if (SpawnedMinion && SpawnedMinion->TeamComponent)
			{
				SpawnedMinion->TeamComponent->SetTeam(Team);
				SpawnedMinion->HomeBase = SpawnPoint;
				SpawnedMinion->ChangeMesh();
				SpawnedMinions.AddUnique(SpawnedMinion);
				SpawnedMinion->OnDestroyed.AddDynamic(this, &AMinionsGroupPawn::OnMinionDeath);
				SpawnedMinion->BindOnAttackTarget(this);
			}
		}
	}
}

bool AMinionsGroupPawn::SetupDetectedEnemy(bool bEnemyBoolDetection, AActor* Actor)
{
	if (!bEnemyBoolDetection) {return true;}
	bInCombat = true; 

	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsBool(InCombatKey, bInCombat);
			AttackTarget = Cast<APawn>(Actor);
			CallOnAttackTargetSet();
			BindEnemyDestroy(Actor);
		}
	}
	return false;
}

void AMinionsGroupPawn::OnEnemyDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor)
	{
		UnBindBindEnemyDestroy(DestroyedActor);
	}
	
	bInCombat = false;
	AttackTarget = nullptr;

	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsBool(InCombatKey, bInCombat);
		}
	}
	
	OnAttackTargetSet.Broadcast(nullptr);
}

void AMinionsGroupPawn::CallOnAttackTargetSet()
{
	OnAttackTargetSet.Broadcast(AttackTarget);
}

void AMinionsGroupPawn::OnMinionDeath(AActor* DeadMinion)
{
	AttackTarget = nullptr;
	AMinionBase* DeadMinionRef = Cast<AMinionBase>(DeadMinion);
	if (DeadMinionRef)
	{
		SpawnedMinions.RemoveSwap(DeadMinionRef);
	}
}

void AMinionsGroupPawn::BindEnemyDestroy(AActor* ActorToBind)
{
	if (ActorToBind && !ActorToBind->IsPendingKillPending())
	{
		ActorToBind->OnDestroyed.AddDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);
	}
}

void AMinionsGroupPawn::UnBindBindEnemyDestroy(AActor* ActorToUnBind)
{
	ActorToUnBind->OnDestroyed.RemoveDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);
}

EGameTeam AMinionsGroupPawn::GetTeamInterface_Implementation() const
{
	return Team;
}
