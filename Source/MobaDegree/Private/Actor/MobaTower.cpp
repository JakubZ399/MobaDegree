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
    
	TowerRadius->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAggroRangeBeginOverlap);
	TowerRadius->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAggroRangeEndOverlap);

	ProjectileSpawnerTransform = ProjectileSpawner->GetComponentTransform();
    
	if (TowerAttackClass && HasAuthority())
	{
		GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(TowerAttackClass, 1));
	}
}

void AMobaTower::OnAggroRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!AttackComponent->GetAttackTarget() && Cast<APawn>(OtherActor) && !bIsAttacking)
	{
		if (IMobaTeamInterface* MobaTeamInterface = Cast<IMobaTeamInterface>(OtherActor))
			if (MobaTeamInterface && MobaTeamInterface->Execute_GetTeamInterface(OtherActor) != TeamComponent->GetTeam())
			{
				AttackComponent->SetAttackTarget(OtherActor);
				bIsAttacking = true;
				SpawnTowerShot();
			}
	}
}

void AMobaTower::OnAggroRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AttackComponent->GetAttackTarget() && AttackComponent->GetAttackTarget() == OtherActor)
	{
		AttackComponent->SetAttackTarget(nullptr);
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