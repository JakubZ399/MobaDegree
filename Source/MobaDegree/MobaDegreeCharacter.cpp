// 2025 Jakub Żurawik. All Rights Reserved.

#include "MobaDegreeCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Component/HealthComponent.h"
#include "Component/TeamComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/MobaAbilitySystemComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Navigation/PathFollowingComponent.h"
#include "Player/MobaPlayerState.h"

AMobaDegreeCharacter::AMobaDegreeCharacter()
{
	bAlwaysRelevant = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetIsReplicated(true);
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetClass(HealthBarWidgetClass);
	HealthBarWidget->SetDrawAtDesiredSize(true);
}

UAbilitySystemComponent* AMobaDegreeCharacter::GetAbilitySystemComponent() const
{
	if (!MobaPlayerState) return nullptr;
	
	return MobaPlayerState->GetAbilitySystemComponent();
}

EGameTeam AMobaDegreeCharacter::GetTeamInterface_Implementation() const
{
	if (!MobaPlayerState) return EGameTeam::None;
	
	return MobaPlayerState->GetTeamComponent()->GetTeam();
}

void AMobaDegreeCharacter::BeginPlay()
{
	Super::BeginPlay();

	MobaPlayerState = Cast<AMobaPlayerState>(GetPlayerState());
}

void AMobaDegreeCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AMobaDegreeCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	MobaPlayerState = Cast<AMobaPlayerState>(GetPlayerState());
	
	InitAbilityActorInfo();
	InitializeAttribute();
}

void AMobaDegreeCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	MobaPlayerState = Cast<AMobaPlayerState>(GetPlayerState());
	
	InitAbilityActorInfo();
	InitializeAttribute();
}

void AMobaDegreeCharacter::InitializeAttribute()
{
	if (!AbilitySystemComponent) return;
	if (!InitEffect) return;
	
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1, EffectContextHandle);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	
	if (HealthComponent && HealthBarWidget && HealthBarWidget->GetWidget())
	{
		HealthComponent->SetHealthBarWidgetFromOwner(HealthBarWidget);
		HealthComponent->HealthBarInitialization();
		HealthComponent->RefreshHealthBar();
	}
}

void AMobaDegreeCharacter::InitAbilityActorInfo()
{
	if (GetAbilitySystemComponent() && MobaPlayerState)
	{
		AbilitySystemComponent = Cast<UMobaAbilitySystemComponent>(MobaPlayerState->GetAbilitySystemComponent());

		AttributeSet = MobaPlayerState->GetMobaAttributeSet();
		
		GetAbilitySystemComponent()->InitAbilityActorInfo(MobaPlayerState, this);
	}
}

void AMobaDegreeCharacter::MoveToLocation(FVector Location)
{
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), Location);
}