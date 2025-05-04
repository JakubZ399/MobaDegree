// 2025 Jakub Żurawik. All Rights Reserved.

#include "Minions/MinionsGroupPawn.h"

#include "AIController.h"
#include "Actor/MobaTower.h"
#include "BehaviorTree/BlackboardComponent.h"
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
	if (bInCombat) return;
	
	TargetsInRange = FindActorsInRange();
	if (TargetsInRange.Num() > 0)
	{
		for (AActor* TargetActor : TargetsInRange)
		{
			//Tower
			if (TargetActor->IsA(AMobaTower::StaticClass()))
			{
				AMobaTower* TargetMobaTower = Cast<AMobaTower>(TargetActor);
				SetupDetectedEnemy(bDetectTower, TargetMobaTower);
				BindTowerEnemy(TargetMobaTower);
				return;
			}
			//Minion Group
			if (AMinionsGroupPawn* TargetGroup = Cast<AMinionsGroupPawn>(TargetActor))
			{
				SetupDetectedEnemy(bDetectMinion, TargetGroup);
				BindGroupEnemy(TargetGroup);
				return;
			}
			//Player
			if (TargetActor->IsA(AMobaDegreeCharacter::StaticClass()))
			{
				AMobaDegreeCharacter* TargetCharacter = Cast<AMobaDegreeCharacter>(TargetActor);
				SetupDetectedEnemy(bDetectPlayers, TargetCharacter);
				BindPlayerEnemy(TargetCharacter);
				return;
			}
		}
	}
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
	bool bSuccess = GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);
	
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
						DrawDebugSphere(GetWorld(), GetActorLocation(), ScanRadius, 16, FColor::Red, false, ScanTime);
					}
				}
			}
		}
		
		DrawDebugSphere(GetWorld(), GetActorLocation(), ScanRadius, 16, FColor::Green, false, ScanTime);
	}
	else
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ScanRadius, 16, FColor::Yellow, false, ScanTime);
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



bool AMinionsGroupPawn::SetupDetectedEnemy(bool bEnemyBoolDetection, APawn* Pawn)
{
	if (!bEnemyBoolDetection) {return true;}
	bInCombat = true; 

	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsBool(InCombatKey, bInCombat);
		}
	}
	return false;
}

void AMinionsGroupPawn::OnEnemyDestroyed(AActor* DestroyedActor)
{
	bInCombat = false;

	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsBool(InCombatKey, bInCombat);
		}
	}
	
	OnAttackTargetSet.Broadcast(nullptr);
}

void AMinionsGroupPawn::OnGroupDeathCallback()
{
	bInCombat = false;

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

void AMinionsGroupPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnGroupDeath.Broadcast();
}

EGameTeam AMinionsGroupPawn::GetTeamInterface_Implementation() const
{
	return Team;
}

void AMinionsGroupPawn::BindTowerEnemy(AMobaTower* Tower)
{
	Tower->OnDestroyed.AddDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);
}

void AMinionsGroupPawn::BindGroupEnemy(AMinionsGroupPawn* OtherGroup)
{
	OtherGroup->OnGroupDeath.AddDynamic(this, &ThisClass::OnGroupDeathCallback);
}

void AMinionsGroupPawn::BindPlayerEnemy(AMobaDegreeCharacter* EnemyPlayer)
{
	EnemyPlayer->OnDestroyed.AddDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);
}

void AMinionsGroupPawn::UnBindTowerEnemy(AMobaTower* Tower)
{
	Tower->OnDestroyed.RemoveDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);
}

void AMinionsGroupPawn::UnBindGroupEnemy(AMinionsGroupPawn* OtherGroup)
{
	OtherGroup->OnGroupDeath.RemoveDynamic(this, &ThisClass::OnGroupDeathCallback);
}

void AMinionsGroupPawn::UnBindPlayerEnemy(AMobaDegreeCharacter* EnemyPlayer)
{
	EnemyPlayer->OnDestroyed.RemoveDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);
}