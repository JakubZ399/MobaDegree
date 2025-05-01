// 2025 Jakub Żurawik. All Rights Reserved.


#include "Actor/MobaTower.h"
#include "GameplayEffectExtension.h"
#include "Actor/Tower/TowerShot.h"
#include "Component/AttackComponent.h"
#include "Components/SphereComponent.h"
#include "Component/TeamComponent.h"


AMobaTower::AMobaTower()
{
	PrimaryActorTick.bCanEverTick = true;

	TowerRadius = CreateDefaultSubobject<USphereComponent>("Tower Radius");
	TowerRadius->SetupAttachment(GetRootComponent());

	AttackComponent = CreateDefaultSubobject<UAttackComponent>("Attack Component");
	AttackComponent->SetIsReplicated(true);

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>("Projectile Spawner");
	ProjectileSpawner->SetupAttachment(GetRootComponent());
}

void AMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMobaTower::BeginPlay()
{
	Super::BeginPlay();
    
	TowerRadius->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTargetEnteredRange);
	TowerRadius->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnTargetExitedRange);

	ProjectileSpawnerTransform = ProjectileSpawner->GetComponentTransform();
    
	if (TowerAttackClass && HasAuthority())
	{
		GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(TowerAttackClass, 1));
	}
}

void AMobaTower::OnTargetEnteredRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()))
	{
		EGameTeam ActorTeam = IMobaTeamInterface::Execute_GetTeamInterface(OtherActor);

		if (ActorTeam != GetTeamInterface())
		{
			TargetsInRange.AddUnique(OtherActor);

			OtherActor->OnDestroyed.AddDynamic(this, &AMobaTower::OnTargetDestroyed);

			if (!AttackComponent->GetAttackTarget())
			{
				AttackComponent->SetAttackTarget(OtherActor);
				bIsAttacking = true;
				SpawnTowerShot();
			}
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
	if (TargetsInRange.Num() > 0)
	{
		AttackComponent->SetAttackTarget(TargetsInRange[0]);
		SpawnTowerShot();
	}
	else
	{
		AttackComponent->SetAttackTarget(nullptr);
		//StopAttacking();
	}
}

void AMobaTower::SpawnTowerShot()
{
	if (GetWorld() && AttackComponent && AttackComponent->AttackTarget && TowerShotClass)
	{
		if (ATowerShot* TowerShot = GetWorld()->SpawnActor<ATowerShot>(TowerShotClass, GetProjectileSpawnerTransform()))
		{
			TowerShot->AttackTarget(AttackComponent->AttackTarget);
			
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(
				TimerHandle,
				[this]()
				{
					bIsAttacking = false;
					SpawnTowerShot();
				},
				2.f,
				false
			);
		}
	}
}

float AMobaTower::GetAttributeTower(FGameplayAttribute AttributeType)
{
	bool Found;
	return GetAbilitySystemComponent()->GetGameplayAttributeValue(AttributeType, Found);
}