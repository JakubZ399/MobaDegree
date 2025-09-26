// 2025 Jakub Żurawik. All Rights Reserved.


#include "Actor/MobaTower.h"
#include "GameplayEffectExtension.h"
#include "Actor/Tower/TowerShot.h"
#include "Component/AttackComponent.h"
#include "Components/SphereComponent.h"
#include "Component/TeamComponent.h"
#include "Minions/MinionsGroupPawn.h"
#include "Net/UnrealNetwork.h"


AMobaTower::AMobaTower()
{
	PrimaryActorTick.bCanEverTick = true;

	TowerRadius = CreateDefaultSubobject<USphereComponent>("Tower Radius");
	TowerRadius->SetupAttachment(GetRootComponent());

	TowerMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Tower Mesh");
	TowerMesh->SetupAttachment(GetRootComponent());

	AttackComponent = CreateDefaultSubobject<UAttackComponent>("Attack Component");
	AttackComponent->SetIsReplicated(true);

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>("Projectile Spawner");
	ProjectileSpawner->SetupAttachment(GetRootComponent());
}

void AMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAiming(DeltaTime);

	if (bAiming)
	{
		if (AlphaRotation >= 1.f)
		{
			AlphaRotation = 1.f;
		}
		if (AlphaRotation < 1.f)
		{
			AlphaRotation += AlphaRotationSpeed * DeltaTime;
		}
	}
	else
	{
		if (AlphaRotation <= 0.f)
		{
			AlphaRotation = 0.f;
		}
		if (AlphaRotation > 0.f)
		{
			AlphaRotation -= AlphaRotationSpeed * DeltaTime;
		}
	}

	if (bAlphaRotation)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Alpha: %f"), AlphaRotation));
	}

}

void AMobaTower::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMobaTower, bAiming);
	DOREPLIFETIME(AMobaTower, bIsFiring);
	DOREPLIFETIME(AMobaTower, DistanceToTarget);
	DOREPLIFETIME(AMobaTower, AttackTargetPosition);
}

void AMobaTower::UpdateAiming(float DeltaTime)
{
	AActor* Target = AttackComponent ? AttackComponent->GetAttackTarget() : nullptr;
	bAiming = IsValid(Target);

	if (!bAiming)
	{
		DistanceToTarget     = 0.f;
		AttackTargetPosition = FVector::ZeroVector;
		return;
	}

	const FVector AimOrigin = ProjectileSpawner ? ProjectileSpawner->GetComponentLocation()
												: GetActorLocation();

	AttackTargetPosition = Target->GetActorLocation();
	DistanceToTarget     = FVector::Dist(AimOrigin, AttackTargetPosition);
}

void AMobaTower::StartAttackLoop()
{
	if (!HasAuthority()) return;
	if (bIsAttacking)    return;

	bIsAttacking = true;

	SpawnTowerShot();

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle, this, &AMobaTower::SpawnTowerShot, TowerAttackTime, true);
}

void AMobaTower::StopAttackLoop()
{
	if (!HasAuthority()) return;
	if (!bIsAttacking)   return;

	bIsAttacking = false;
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
}

void AMobaTower::ResetIsFiring()
{
	bIsFiring = false;
	ForceNetUpdate();
}

void AMobaTower::BeginPlay()
{
	Super::BeginPlay();
    
	TowerRadius->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTargetEnteredRange);
	TowerRadius->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnTargetExitedRange);
    
	if (TowerAttackClass && HasAuthority())
	{
		GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(TowerAttackClass, 1));
	}
}

void AMobaTower::OnTargetEnteredRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()))
		return;

	if (Cast<AMinionsGroupPawn>(OtherActor)) return;

	EGameTeam ActorTeam = IMobaTeamInterface::Execute_GetTeamInterface(OtherActor);
	if (ActorTeam != TeamComponent->GetTeam())
	{
		TargetsInRange.AddUnique(OtherActor);
		OtherActor->OnDestroyed.AddDynamic(this, &AMobaTower::OnTargetDestroyed);
		
		if (!AttackComponent->GetAttackTarget())
		{
			AttackComponent->SetAttackTarget(OtherActor);

			bAiming = true;
			StartAttackLoop();
		}
	}
}

void AMobaTower::OnTargetExitedRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		TargetsInRange.Remove(OtherActor);
		
		OtherActor->OnDestroyed.RemoveDynamic(this, &AMobaTower::OnTargetDestroyed);
		
		if (OtherActor == AttackComponent->GetAttackTarget())
		{
			SelectNextTarget();
		}
	}
}

void AMobaTower::OnTargetDestroyed(AActor* DestroyedActor)
{
	TargetsInRange.Remove(DestroyedActor);

	if (DestroyedActor == AttackComponent->GetAttackTarget())
	{
		SelectNextTarget();
	}
}

void AMobaTower::SelectNextTarget()
{
	AActor* Best = nullptr;
	float BestSq = TNumericLimits<float>::Max();
	const FVector Origin = GetActorLocation();

	for (AActor* Candidate : TargetsInRange)
	{
		if (!IsValid(Candidate)) continue;
		const float DistSq = FVector::DistSquared(Candidate->GetActorLocation(), Origin);
		if (DistSq < BestSq)
		{
			BestSq = DistSq;
			Best = Candidate;
		}
	}

	AttackComponent->SetAttackTarget(Best);

	if (Best)
	{
		bAiming = true;
		if (!bIsAttacking) StartAttackLoop();
	}
	else
	{
		bAiming = false;
		StopAttackLoop();
	}
}

void AMobaTower::SpawnTowerShot()
{
	if (!HasAuthority()) return;

	AActor* Target = AttackComponent ? AttackComponent->GetAttackTarget() : nullptr;
	if (!Target)
	{
		StopAttackLoop();
		return;
	}
	
	bIsFiring = true;
	ForceNetUpdate();
	GetWorldTimerManager().SetTimerForNextTick(this, &AMobaTower::ResetIsFiring);
	
	ActivateAbilityAttack();
}

float AMobaTower::GetAttributeTower(FGameplayAttribute AttributeType)
{
	bool Found;
	return GetAbilitySystemComponent()->GetGameplayAttributeValue(AttributeType, Found);
}