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

void AMobaDegreePlayerController::OnSetDestinationReleased()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ClickDuration = CurrentTime - StartClickTime;
    
    if (ClickDuration < ShortPressThreshold)
    {
        FHitResult HitPawnResult;
        bool bHitSuccessfulHitPawn = GetHitResultUnderCursor(ECC_Pawn, false, HitPawnResult);

        if (bHitSuccessfulHitPawn && PlayerCharacter)
        {
            AActor* HitActor = HitPawnResult.GetActor();
            
            if (HitActor && HitActor != PlayerCharacter)
            {
                APawn* HitPawn = Cast<APawn>(HitActor);
                if (HitPawn)
                {
                    IMobaTeamInterface* TeamInterface = Cast<IMobaTeamInterface>(HitPawn);
                    IMobaTeamInterface* PlayerTeamInterface = Cast<IMobaTeamInterface>(PlayerCharacter);
                    
                    if (TeamInterface && PlayerTeamInterface && 
                        TeamInterface->Execute_GetTeamInterface(HitPawn) != PlayerTeamInterface->Execute_GetTeamInterface(PlayerCharacter))
                    {
                        bPawnClicked = true;
                        ProcessTargetSelection(HitActor);
                        return;
                    }
                }
            }
        }
        
        FHitResult Hit;
        bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
        
        if (bHitSuccessful)
        {
            CachedDestination = Hit.Location;
            if (PlayerCharacter && PlayerCharacter->AttackTarget)
            {
                ChangeOutline(PlayerCharacter->AttackTarget, false);
                Server_ClearTarget();
            }
            
            SpawnCursorFX(CachedDestination);
            
            UE_LOG(LogTemplateCharacter, Log, TEXT("Click detected at location: %s"), *CachedDestination.ToString());
        }
    }
    
    bPawnClicked = false;
}

void AMobaDegreePlayerController::ProcessTargetSelection(AActor* TargetActor)
{
    if (PlayerCharacter && PlayerCharacter->AttackTarget == TargetActor)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("Already targeting"));
        return;
    }

    AActor* OldTarget = PlayerCharacter ? PlayerCharacter->AttackTarget : nullptr;
    
    if (OldTarget)
    {
        ChangeOutline(OldTarget, false);
    }
    
    ChangeOutline(TargetActor, true);

    Server_SelectTarget(TargetActor);
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
    if (OutlineActor && IsLocalController())
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("ChangeOutline: Actor=%s, Show=%s"), 
            *OutlineActor->GetName(), ShowOutline ? TEXT("true") : TEXT("false"));
            
        if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OutlineActor))
        {
            UE_LOG(LogTemplateCharacter, Log, TEXT("Calling ShowOutline interface"));
            MobaInteraction->Execute_ShowOutline(OutlineActor, ShowOutline);
        }
        else
        {
            UE_LOG(LogTemplateCharacter, Warning, TEXT("Actor does not implement IMobaInteraction"));
        }
    }
    else if (!OutlineActor)
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("ChangeOutline called with null actor"));
    }
}

void AMobaDegreePlayerController::Server_SelectTarget_Implementation(AActor* Target)
{
    if (!IsValid(PlayerCharacter) || !IsValid(Target)) 
    {
        return;
    }
    
    AActor* OldTarget = PlayerCharacter->AttackTarget;
    PlayerCharacter->OldAttackTarget = OldTarget;
    PlayerCharacter->AttackTarget = Target;

    Client_OnTargetChanged(OldTarget, Target);
    
    UE_LOG(LogTemplateCharacter, Log, TEXT("Target selected: %s"), *Target->GetName());
}

void AMobaDegreePlayerController::Client_OnTargetChanged_Implementation(AActor* OldTarget, AActor* NewTarget)
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("Client_OnTargetChanged: OldTarget=%s, NewTarget=%s"), 
        OldTarget ? *OldTarget->GetName() : TEXT("None"),
        NewTarget ? *NewTarget->GetName() : TEXT("None"));

    if (OldTarget)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("Disabling outline for: %s"), *OldTarget->GetName());
        ChangeOutline(OldTarget, false);
    }

    if (NewTarget)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("Enabling outline for: %s"), *NewTarget->GetName());
        ChangeOutline(NewTarget, true);
    }
}

void AMobaDegreePlayerController::Server_ClearTarget_Implementation()
{
    if (!IsValid(PlayerCharacter) || !PlayerCharacter->AttackTarget) 
    {
        return;
    }
    
    AActor* OldTarget = PlayerCharacter->AttackTarget;
    PlayerCharacter->OldAttackTarget = OldTarget;
    PlayerCharacter->AttackTarget = nullptr;

    Client_OnTargetChanged(OldTarget, nullptr);
}