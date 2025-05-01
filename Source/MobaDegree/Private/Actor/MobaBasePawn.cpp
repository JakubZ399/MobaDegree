// 2025 Jakub Żurawik. All Rights Reserved.


#include "Actor/MobaBasePawn.h"

#include "AbilitySystemComponent.h"
#include "Component/HealthComponent.h"
#include "Component/TeamComponent.h"
#include "Components/CapsuleComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Net/UnrealNetwork.h"

AMobaBasePawn::AMobaBasePawn()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	SetRootComponent(CapsuleComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pawn Mesh"));
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetupAttachment(GetRootComponent());

	MobaAbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	MobaAbilitySystemComponent->SetIsReplicated(true);
	MobaAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	MobaAttributeSet = CreateDefaultSubobject<UMobaAttributeSet>(TEXT("AttributeSet"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	HealthComponent->SetIsReplicated(true);

	TeamComponent = CreateDefaultSubobject<UTeamComponent>("TeamComponent");
	TeamComponent->SetIsReplicated(true);

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>("HealthBar Widget");
	HealthBarWidget->SetIsReplicated(true);
	HealthBarWidget->SetupAttachment(GetRootComponent());
	HealthBarWidget->SetWidgetClass(HealthBarWidgetClass);
	HealthBarWidget->SetDrawAtDesiredSize(true);
}

void AMobaBasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool Found = false;
	float HealthValue = MobaAbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetHealthAttribute(), Found);
	
	if (HealthValue <= 0)
	{
		Destroy();
	}
}

void AMobaBasePawn::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AMobaBasePawn, bAttributeInitialized, COND_None, REPNOTIFY_Always);
}

void AMobaBasePawn::ShowOutline_Implementation(bool EnableOutline, int32 OutlineColor)
{
	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(EnableOutline);
		MeshComponent->SetCustomDepthStencilValue(OutlineColor);
	}
}

void AMobaBasePawn::BeginPlay()
{
	Super::BeginPlay();

	check (MobaAbilitySystemComponent);
	check (MobaAttributeSet);

	InitializeAttributes();

	if (HasAuthority())
	{
		bAttributeInitialized = true;
	}

	if (HealthComponent && HealthBarWidget && HealthBarWidget->GetWidget())
	{
		HealthComponent->SetHealthBarWidgetFromOwner(HealthBarWidget);
		HealthComponent->HealthBarInitialization();
		HealthComponent->SetHealthBarColor();
	}
}

void AMobaBasePawn::InitializeAttributes()
{
	if (MobaAbilitySystemComponent && MobaAttributeSet && InitEffect)
	{
		MobaAbilitySystemComponent->InitAbilityActorInfo(this, this);

		FGameplayEffectContextHandle EffectContextHandle = MobaAbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = MobaAbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1 , EffectContextHandle);
		MobaAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AMobaBasePawn::OnRep_bAttributeInitialized()
{
	InitializeAttributes();
}

#pragma region GettersAndSetters

UAbilitySystemComponent* AMobaBasePawn::GetAbilitySystemComponent() const
{
	if (MobaAbilitySystemComponent)
	{
		return MobaAbilitySystemComponent;
	}
	return nullptr;
}

EGameTeam AMobaBasePawn::GetTeamInterface_Implementation() const
{
	if (TeamComponent)
	{
		return TeamComponent->GetTeam();
	}
	return EGameTeam::None;
}

#pragma endregion GettersAndSetters



