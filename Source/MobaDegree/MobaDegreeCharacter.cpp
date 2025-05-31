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
#include "MobaDegreePlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
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
}

void AMobaDegreeCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    
    if (AbilitySystemComponent && AttributeSet)
    {
        bool bFound = false;
        CurrentAttackRange = AbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetAttackRangeAttribute(), bFound);
    }
    
    HandleCombat(DeltaSeconds);
}

void AMobaDegreeCharacter::HandleCombat(float DeltaTime)
{
    if (CombatState != ECharacterCombatState::Casting)
    {
        if (AttackTarget && !IsValidAttackTarget(AttackTarget))
        {
            ClearAttackTarget();
            Server_SetCombatState(ECharacterCombatState::Idle);
            return;
        }
    
        if (!AttackTarget)
        {
            if (CombatState != ECharacterCombatState::Idle)
            {
                Server_SetCombatState(ECharacterCombatState::Idle);
            }
            return;
        }
    
        UpdateCombatState();
    }
    
    switch (CombatState)
    {
    case ECharacterCombatState::Moving:
        MoveToTarget();
        RotateToTarget(DeltaTime);
        break;
            
    case ECharacterCombatState::Attacking:
        RotateToTarget(DeltaTime);
        PerformAttack();
        break;

    case ECharacterCombatState::Casting:
        RotateToTarget(DeltaTime);
        break;
        
    }
}

void AMobaDegreeCharacter::UpdateCombatState()
{
    if (!AttackTarget || !IsValidAttackTarget(AttackTarget))
    {
        Server_SetCombatState(ECharacterCombatState::Idle);
        ClearAttackTarget();
        return;
    }
    
    ECharacterCombatState NewState = IsInAttackRange() ? 
        ECharacterCombatState::Attacking : 
        ECharacterCombatState::Moving;
    
    if (NewState != CombatState)
    {
        Server_SetCombatState(NewState);
    }
}

void AMobaDegreeCharacter::Server_SetCombatState_Implementation(ECharacterCombatState NewState)
{
    CombatState = NewState;
    
    if (CombatState == ECharacterCombatState::Attacking)
    {
        GetCharacterMovement()->StopMovementImmediately();
    }
}

void AMobaDegreeCharacter::MoveToTarget()
{
    if (!AttackTarget || !IsValidAttackTarget(AttackTarget)) return;
    
    FVector Direction = (AttackTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Direction);
}

void AMobaDegreeCharacter::RotateToTarget(float DeltaTime)
{
    if (!AttackTarget || !IsValidAttackTarget(AttackTarget)) return;
    
    FVector LookDirection = AttackTarget->GetActorLocation() - GetActorLocation();
    LookDirection.Z = 0.0f;
    
    if (!LookDirection.IsNearlyZero())
    {
        FRotator TargetRotation = LookDirection.Rotation();
        FRotator CurrentRotation = GetActorRotation();
        
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
        SetActorRotation(NewRotation);
    }
}

void AMobaDegreeCharacter::PerformAttack()
{
    if (!CanPerformAction()) return;
    
    Server_PerformAttack();
}

void AMobaDegreeCharacter::Server_PerformAttack_Implementation()
{
    if (!CanPerformAction() || !AbilitySystemComponent || !PrimaryAttackAbility) return;
    
    AbilitySystemComponent->TryActivateAbilityByClass(PrimaryAttackAbility);
}

bool AMobaDegreeCharacter::IsInAttackRange() const
{
    if (!AttackTarget || !IsValidAttackTarget(AttackTarget)) return false;
    
    float Distance = FVector::Dist2D(GetActorLocation(), AttackTarget->GetActorLocation());
    return Distance <= CurrentAttackRange;
}

bool AMobaDegreeCharacter::CanPerformAction() const
{
    return CombatState != ECharacterCombatState::Casting;
}

bool AMobaDegreeCharacter::IsValidAttackTarget(AActor* Target) const
{
    if (!Target || !IsValid(Target))
    {
        return false;
    }
    
    IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);
    if (ASCInterface && ASCInterface->GetAbilitySystemComponent())
    {
        UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
        const UMobaAttributeSet* TargetAttributeSet = Cast<UMobaAttributeSet>(TargetASC->GetAttributeSet(UMobaAttributeSet::StaticClass()));
        
        if (TargetAttributeSet)
        {
            float TargetHealth = TargetAttributeSet->GetHealth();
            return TargetHealth > 0.0f;
        }
    }
    
    // If we can't check health, just verify the actor is valid
    return IsValid(Target);
}

void AMobaDegreeCharacter::SetAttackTarget(AActor* Target)
{
    if (AttackTarget && Target != AttackTarget)
    {
        if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(AttackTarget))
        {
            MobaInteraction->Execute_ShowOutline(AttackTarget, false, 0);
        }
    }
    
    AttackTarget = Target;
    OldAttackTarget = Target;
    
    if (AttackTarget)
    {
        Server_SetCombatState(ECharacterCombatState::Moving);
    }
    else
    {
        Server_SetCombatState(ECharacterCombatState::Idle);
    }
}

void AMobaDegreeCharacter::ClearAttackTarget()
{
    if (AttackTarget)
    {
        if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(AttackTarget))
        {
            MobaInteraction->Execute_ShowOutline(AttackTarget, false, 0);
        }
    }
    
    SetAttackTarget(nullptr);
    
    if (AMobaDegreePlayerController* MobaController = Cast<AMobaDegreePlayerController>(GetController()))
    {
        MobaController->ClearMovementSpline();
    }
}

void AMobaDegreeCharacter::InterruptCombat()
{
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->CancelAllAbilities();
    }
    
    ClearAttackTarget();
    Server_SetCombatState(ECharacterCombatState::Idle);
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