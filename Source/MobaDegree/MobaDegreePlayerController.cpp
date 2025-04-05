// 2025 Jakub Żurawik. All Rights Reserved.

#include "MobaDegreePlayerController.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "MobaDegreeCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMobaDegreePlayerController::AMobaDegreePlayerController()
{
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
    CachedDestination = FVector::ZeroVector;
}

void AMobaDegreePlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
    }
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
        EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AMobaDegreePlayerController::OnInputStarted);
        EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AMobaDegreePlayerController::OnSetDestinationReleased);
        EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AMobaDegreePlayerController::OnSetDestinationReleased);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system."), *GetNameSafe(this));
    }
}

void AMobaDegreePlayerController::OnInputStarted()
{
    StartClickTime = GetWorld()->GetTimeSeconds();
    StopMovement();
}

void AMobaDegreePlayerController::OnSetDestinationTriggered()
{
}

void AMobaDegreePlayerController::OnSetDestinationReleased()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ClickDuration = CurrentTime - StartClickTime;

    if (ClickDuration < ShortPressThreshold)
    {
        FHitResult Hit;
        bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

        if (bHitSuccessful)
        {
            CachedDestination = Hit.Location;
            SpawnCursorFX(CachedDestination);

            UE_LOG(LogTemplateCharacter, Log, TEXT("Click detected at location: %s"), *CachedDestination.ToString());
        }
    }
    bPawnClicked = false;
}

void AMobaDegreePlayerController::SpawnCursorFX(const FVector& Location)
{
    if (FXCursor && IsLocalController())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
    }
}

void AMobaDegreePlayerController::ChangeOutline(AActor* OutlineActor, bool ShowOutline)
{
    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OutlineActor))
    {
        MobaInteraction->Execute_ShowOutline(OutlineActor, ShowOutline);
    }
}