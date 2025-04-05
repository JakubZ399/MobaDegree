// 2025 Jakub Żurawik. All Rights Reserved.

#include "Minions/MinionsGroupPawn.h"

#include "AIController.h"
#include "Actor/MobaTower.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/TeamComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetMathLibrary.h"
#include "Minions/MinionBase.h"
#include "MobaDegree/MobaDegreeCharacter.h"
#include "Perception/PawnSensingComponent.h"

AMinionsGroupPawn::AMinionsGroupPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnPointsMinion = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPointMinion"));
	SetRootComponent(SpawnPointsMinion);
	
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = 300.f;
	
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensingComponent->SightRadius = 1000.f;
	PawnSensingComponent->SetPeripheralVisionAngle(60.f);

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

	PawnSensingComponent->OnSeePawn.AddDynamic(this, &AMinionsGroupPawn::OnSeePawn);

	PawnSensingComponent->SensingInterval = 0.25f;
	PawnSensingComponent->bOnlySensePlayers = false;
	PawnSensingComponent->bSeePawns = true;

	AttackTarget = nullptr;
}

void AMinionsGroupPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnGroupDeath.Broadcast();
}

void AMinionsGroupPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMinionsGroupPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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
	if (bDebugRespawnMelee)
	{
		MinionsSpawn(MeleeMinionClass, SpawnPointMinionMelee);
	}
	if (bDebugRespawnRanged)
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

			if (SpawnedMinion)
			{
				SpawnedMinion->TeamComponent->SetTeam(Team);
				SpawnedMinion->HomeBase = SpawnPoint;
				SpawnedMinion->ChangeMesh();
				SpawnedMinions.AddUnique(SpawnedMinion);
				//SpawnedMinion->OnMinionDeath.AddDynamic(this, &AMinionsGroupPawn::OnMinionDeath);
				SpawnedMinion->OnDestroyed.AddDynamic(this, &AMinionsGroupPawn::OnMinionDeath);
				SpawnedMinion->BindOnAttackTarget(this);
			}
		}
	}
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

void AMinionsGroupPawn::OnSeePawn(APawn* Pawn)
{
	IMobaTeamInterface* TeamInterface = Cast<IMobaTeamInterface>(Pawn);
	if (TeamInterface && TeamInterface->Execute_GetTeamInterface(Pawn) != Team && !bInCombat)
	{
		//TODO::Agressive Enemy :: Prio.
		
		// Tower
		if (AMobaTower* Tower = Cast<AMobaTower>(Pawn))
		{
			SetupDetectedEnemy(bDetectTower, Pawn);

			Tower->OnDestroyed.AddDynamic(this, &AMinionsGroupPawn::OnEnemyDestroyed);

			return;
		}

		// Minions - other group
		if (AMinionsGroupPawn* OtherGroup = Cast<AMinionsGroupPawn>(Pawn))
		{
			SetupDetectedEnemy(bDetectMinion, Pawn);

			OtherGroup->OnGroupDeath.AddDynamic(this, &ThisClass::OnGroupDeathCallback);
			
			return;
		}

		//Enemy character
		if (AMobaDegreeCharacter* EnemyCharacter = Cast<AMobaDegreeCharacter>(Pawn))
		{
			SetupDetectedEnemy(bDetectPlayers, Pawn);

			//TODO:: Callback to character
			
			return;
		}
	}
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

EGameTeam AMinionsGroupPawn::GetTeamInterface_Implementation() const
{
	return Team;
}

