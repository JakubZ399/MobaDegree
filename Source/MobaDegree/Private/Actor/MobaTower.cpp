// 2025 Jakub Żurawik. All Rights Reserved.


#include "Actor/MobaTower.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Actor/Tower/TowerShot.h"
#include "Component/AttackComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Component/TeamComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Interfaces/UIInterface.h"

AMobaTower::AMobaTower()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(CapsuleComponent);

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>("Tower Mesh");
	TowerMesh->SetupAttachment(GetRootComponent());

	TowerRadius = CreateDefaultSubobject<USphereComponent>("Tower Radius");
	TowerRadius->SetupAttachment(GetRootComponent());

	AttackComponent = CreateDefaultSubobject<UAttackComponent>("Attack Component");
	AttackComponent->SetIsReplicated(true);

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>("Projectile Spawner");
	ProjectileSpawner->SetupAttachment(GetRootComponent());

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMobaAttributeSet>(TEXT("AttributeSet"));

	TeamComponent = CreateDefaultSubobject<UTeamComponent>("TeamComponent");
	TeamComponent->SetIsReplicated(true);

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>("Health Bar Widget");
	HealthBarWidget->SetupAttachment(GetRootComponent());
	HealthBarWidget->SetIsReplicated(true);
	HealthBarWidget->SetWidgetClass(HealthBarWidgetClass);
	
}

void AMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HealthBarWidgetInterface && bIsLoaded)
	{
		float MaxHealth = GetAttributeTower(UMobaAttributeSet::GetMaxHealthAttribute());
		float CurrentHealth = (MaxHealth > 0.f) ? GetAttributeTower(UMobaAttributeSet::GetHealthAttribute()) / MaxHealth : 0.f;
		IUIInterface::Execute_SetBarValue(HealthBarWidgetInterface.GetObject(), CurrentHealth);

		if (GetAttributeTower(UMobaAttributeSet::GetHealthAttribute()) <= 0.f)
		{
			Destroy();
		}
	}

}

void AMobaTower::BeginPlay()
{
	Super::BeginPlay();
	
	
	InitializeAttribute();

	checkf(AbilitySystemComponent, TEXT("AbilitySystemComponent is not set on Tower!"));

	TowerRadius->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAggroRangeBeginOverlap);
	TowerRadius->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAggroRangeEndOverlap);

	ProjectileSpawnerTransform = ProjectileSpawner->GetComponentTransform();
	
	bIsLoaded = true;
	

	if (HealthBarWidget)
	{
		if (UWidget* Widget = HealthBarWidget->GetWidget())
		{
			if (Cast<IUIInterface>(Widget))
			{
				HealthBarWidgetInterface = Widget;
				IUIInterface::Execute_SetBarValue(HealthBarWidgetInterface.GetObject(), 1.f);
			}
		}
	}
	if (TowerAttackClass)
	{
		if (HasAuthority())
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(TowerAttackClass, 1));
		}
	}
}

void AMobaTower::InitializeAttribute()
{
	if (AbilitySystemComponent && AttributeSet && InitEffect)
	{
		FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1 , EffectContextHandle);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
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

EGameTeam AMobaTower::GetTeamInterface_Implementation() const
{
	return TeamComponent->GetTeam();
}

float AMobaTower::GetAttributeTower(FGameplayAttribute AttributeType)
{
	bool Found;
	return AbilitySystemComponent->GetGameplayAttributeValue(AttributeType, Found);
}

UAbilitySystemComponent* AMobaTower::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMobaTower::ShowOutline_Implementation(bool EnableOutline)
{
	TowerMesh->SetRenderCustomDepth(EnableOutline);
}
