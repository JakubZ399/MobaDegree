// 2025 Jakub Żurawik. All Rights Reserved.

#include "MobaDegreeCharacter.h"

#include "GameplayAbilitySet.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "MobaDegreePlayerController.h"
#include "Component/HealthComponent.h"
#include "Component/TeamComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/MobaAbilitySystemComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Player/MobaPlayerState.h"
#include "Net/UnrealNetwork.h"

AMobaDegreeCharacter::AMobaDegreeCharacter()
{
    bAlwaysRelevant = true;
    bReplicates = true;
    
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    
    TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCameraComponent->bUsePawnControlRotation = false;
    
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    
    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
    HealthBarWidget->SetIsReplicated(true);
    HealthBarWidget->SetupAttachment(RootComponent);
    HealthBarWidget->SetDrawAtDesiredSize(true);
}

void AMobaDegreeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AMobaDegreeCharacter, CombatState);
    DOREPLIFETIME(AMobaDegreeCharacter, AttackTarget);
}

void AMobaDegreeCharacter::OnRep_CombatState()
{
}

void AMobaDegreeCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    MobaPlayerState = Cast<AMobaPlayerState>(GetPlayerState());

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this]()
    {
        if (HasAuthority())
        {
            if (PrimaryAttackAbility)
            {
                FGameplayAbilitySpec PrimaryAttackAbilitySpec = AbilitySystemComponent->BuildAbilitySpecFromClass(PrimaryAttackAbility);
                AbilitySystemComponent->GiveAbility(PrimaryAttackAbilitySpec);
            }

            if (Ability1)
            {
                FGameplayAbilitySpec Ability1Spec = AbilitySystemComponent->BuildAbilitySpecFromClass(Ability1);
                AbilitySystemComponent->GiveAbility(Ability1Spec);   
            }

            if (AbilityRMBClass)
            {
                FGameplayAbilitySpec AbilityRMBSpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityRMBClass);
                AbilitySystemComponent->GiveAbility(AbilityRMBSpec);
            }

            if (AbilityQClass)
            {
                FGameplayAbilitySpec AbilityQSpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityQClass);
                AbilitySystemComponent->GiveAbility(AbilityQSpec);
            }

            if (AbilityEClass)
            {
                FGameplayAbilitySpec AbilityESpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityEClass);
                AbilitySystemComponent->GiveAbility(AbilityESpec);
            }

            if (AbilityRClass)
            {
                FGameplayAbilitySpec AbilityRSpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityRClass);
                AbilitySystemComponent->GiveAbility(AbilityRSpec);
            }
        }
    },
        1.f, false);
}

void AMobaDegreeCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    
    if (AbilitySystemComponent && AttributeSet)
    {
        bool bFound = false;
        CurrentAttackRange = AbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetAttackRangeAttribute(), bFound);
    }
}

void AMobaDegreeCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    MobaPlayerState = Cast<AMobaPlayerState>(GetPlayerState());
    
    
    InitAbilityActorInfo();
    InitializeAttribute();

    MobaPlayerController = Cast<AMobaDegreePlayerController>(NewController);
    if (MobaPlayerController && AbilitySystemComponent)
    {
        MobaPlayerController->SetAbilitySystemComponent(AbilitySystemComponent);
    }
}

void AMobaDegreeCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    MobaPlayerState = Cast<AMobaPlayerState>(GetPlayerState());
    
    InitAbilityActorInfo();
    InitializeAttribute();
    
    if (MobaPlayerController && AbilitySystemComponent)
    {
        MobaPlayerController->SetAbilitySystemComponent(AbilitySystemComponent);
    }
}

void AMobaDegreeCharacter::InitializeAttribute()
{
    if (!AbilitySystemComponent) return;
    if (!InitEffect) return;
    if (!HealthRegenEffect) return;
    
    FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
    EffectContextHandle.AddSourceObject(this);
    
    FGameplayEffectSpecHandle InitSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1, EffectContextHandle);
    if (InitSpecHandle.IsValid())
    {
        FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*InitSpecHandle.Data.Get());
        
        bool MovementFound = false;
        float MovementSpeed = AbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMovementSpeedAttribute(), MovementFound);
        
        if (MovementFound)
        {
            GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
        }
    }
    
    FGameplayEffectContextHandle RegenContextHandle = AbilitySystemComponent->MakeEffectContext();
    RegenContextHandle.AddSourceObject(this);
    
    FGameplayEffectSpecHandle RegenSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(HealthRegenEffect, 1, RegenContextHandle);
    if (RegenSpecHandle.IsValid())
    {
        FActiveGameplayEffectHandle RegenGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*RegenSpecHandle.Data.Get());
    }
    
    if (HealthComponent && HealthBarWidget && HealthBarWidget->GetWidget())
    {
        HealthComponent->SetHealthBarWidgetFromOwner(HealthBarWidget);
        HealthComponent->HealthBarInitialization();
        HealthComponent->SetHealthBarColor();
        HealthComponent->RefreshHealthBar();
        HealthComponent->RefreshManaBar();
    }
    
    if (HealthComponent && HealthBarWidget && !HealthBarWidget->GetWidget())
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            if (HealthBarWidget->GetWidget())
            {
                HealthComponent->SetHealthBarWidgetFromOwner(HealthBarWidget);
                HealthComponent->HealthBarInitialization();
                HealthComponent->SetHealthBarColor();
                HealthComponent->RefreshHealthBar();
                HealthComponent->RefreshManaBar();
            }
        }, 0.1f, false);
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

void AMobaDegreeCharacter::ActivateRMBAbility()
{
    if (!AbilitySystemComponent && AbilityRMBClass) return;
    AbilitySystemComponent->TryActivateAbilityByClass(AbilityRMBClass);
}

void AMobaDegreeCharacter::ActivateQAbility()
{
    if (!AbilitySystemComponent && AbilityQClass) return;
    AbilitySystemComponent->TryActivateAbilityByClass(AbilityQClass);
}

void AMobaDegreeCharacter::ActivateEAbility()
{
    if (!AbilitySystemComponent && AbilityEClass) return;
    AbilitySystemComponent->TryActivateAbilityByClass(AbilityEClass);
}

void AMobaDegreeCharacter::ActivateRAbility()
{
    if (!AbilitySystemComponent && AbilityRClass) return;
    AbilitySystemComponent->TryActivateAbilityByClass(AbilityRClass);
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

void AMobaDegreeCharacter::ShowOutline_Implementation(bool EnableOutline, int32 OutlineColor)
{
    if (GetMesh())
    {
        GetMesh()->SetRenderCustomDepth(EnableOutline);
        GetMesh()->SetCustomDepthStencilValue(OutlineColor);
    }
}