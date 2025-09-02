// 2025 Jakub Żurawik. All Rights Reserved.

#include "MobaDegreePlayerController.h"
#include "GameFramework/Pawn.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "MobaDegreeCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/SplineComponent.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"
#include "UI/Widget/MobaMainUserWidget.h"


AMobaDegreePlayerController::AMobaDegreePlayerController()
{
    bReplicates = true;
}

void AMobaDegreePlayerController::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
    CreateMainWidget();
}

void AMobaDegreePlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    PlayerCharacter = Cast<AMobaDegreeCharacter>(InPawn);
}

void AMobaDegreePlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();
    PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
}

void AMobaDegreePlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AMobaDegreePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::Look);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::Move);

        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::PerformAttack);

        EnhancedInputComponent->BindAction(AbilityRMBAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::ActivateRMBAbilityCallback);
        EnhancedInputComponent->BindAction(AbilityQAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::ActivateQAbilityCallback);
        EnhancedInputComponent->BindAction(AbilityEAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::ActivateEAbilityCallback);
        EnhancedInputComponent->BindAction(AbilityRAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::ActivateRAbilityCallback);
    }
}

void AMobaDegreePlayerController::Look(const FInputActionValue& Value)
{
    FVector2D LookInput = Value.Get<FVector2D>();

    AddYawInput(LookInput.X);
    AddPitchInput(-LookInput.Y);
}

void AMobaDegreePlayerController::Move(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        FVector2D MovementVector = Value.Get<FVector2D>();

        if (MovementVector.IsNearlyZero())
            return;

        const FRotator ControlRot = GetControlRotation();
        const FRotator YawRot(0, ControlRot.Yaw, 0);

        const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
        const FVector RightDir   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

        ControlledPawn->AddMovementInput(ForwardDir, MovementVector.Y);
        ControlledPawn->AddMovementInput(RightDir, MovementVector.X);
    }
}

void AMobaDegreePlayerController::PerformAttack(const FInputActionValue& Value)
{
    if (!PlayerCharacter) return;
    PlayerCharacter->PerformAttack();
}

void AMobaDegreePlayerController::ActivateRMBAbilityCallback(const FInputActionValue& Value)
{
    if (!PlayerCharacter) return;
    PlayerCharacter->ActivateRMBAbility();
}

void AMobaDegreePlayerController::ActivateQAbilityCallback(const FInputActionValue& Value)
{
    if (!PlayerCharacter) return;
    PlayerCharacter->ActivateQAbility();
}

void AMobaDegreePlayerController::ActivateEAbilityCallback(const FInputActionValue& Value)
{
    if (!PlayerCharacter) return;
    PlayerCharacter->ActivateEAbility();
}

void AMobaDegreePlayerController::ActivateRAbilityCallback(const FInputActionValue& Value)
{
    if (!PlayerCharacter) return;
    PlayerCharacter->ActivateRAbility();
}

FHitResult AMobaDegreePlayerController::TraceFromeScreenCenter(float Distance)
{
    FHitResult HitResult;

    int32 ViewportX, ViewportY;
    GetViewportSize(ViewportX, ViewportY);

    FVector WorldLocation, WorldDirection;
    if (DeprojectScreenPositionToWorld(ViewportX * 0.5f, ViewportY * 0.5f, WorldLocation, WorldDirection))
    {
        FVector Start = WorldLocation;
        FVector End = Start + (WorldDirection * Distance);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(GetPawn());

        GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Visibility,
            Params
        );
    }

    return HitResult;
}

void AMobaDegreePlayerController::CreateMainWidget_Implementation()
{
    if (MainUserWidgetClass)
    {
        MainUserWidget = CreateWidget<UMobaMainUserWidget>(this, MainUserWidgetClass);
        if (MainUserWidget)
        {
            MainUserWidget->AddToViewport();
            InitializeMainWidgetBlueprintVariable();
        }
    }
}
