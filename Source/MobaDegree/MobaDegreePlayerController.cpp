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

void AMobaDegreePlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    PlayerCharacter = Cast<AMobaDegreeCharacter>(InPawn);
    
    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] OnPossess - PlayerCharacter: %s"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"),
        PlayerCharacter ? *PlayerCharacter->GetName() : TEXT("None"));
}

void AMobaDegreePlayerController::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
    
    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] BeginPlay - PlayerCharacter: %s"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"),
        PlayerCharacter ? *PlayerCharacter->GetName() : TEXT("None"));
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
    
    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] OnSetDestinationReleased - Click duration: %f"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
        ClickDuration);
    
    if (ClickDuration < ShortPressThreshold)
    {
        FHitResult HitPawnResult;
        bool bHitSuccessfulHitPawn = GetHitResultUnderCursor(ECC_Pawn, false, HitPawnResult);

        if (bHitSuccessfulHitPawn && PlayerCharacter)
        {
            AActor* HitActor = HitPawnResult.GetActor();
            
            if (HitActor && HitActor != PlayerCharacter)
            {
                UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Hit Actor: %s"), 
                    IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
                    *HitActor->GetName());
                
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
            
            UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Clicked on ground, clearing target. Current target: %s"), 
                IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
                PlayerCharacter && PlayerCharacter->AttackTarget ? *PlayerCharacter->AttackTarget->GetName() : TEXT("None"));

            if (PlayerCharacter && PlayerCharacter->AttackTarget)
            {
                UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Locally disabling outline for: %s"), 
                    IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
                    *PlayerCharacter->AttackTarget->GetName());

                if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(PlayerCharacter->AttackTarget))
                {
                    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Directly executing ShowOutline(false)"), 
                        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"));
                    MobaInteraction->Execute_ShowOutline(PlayerCharacter->AttackTarget, false);
                }
            }

            Server_ClearTarget();
            
            SpawnCursorFX(CachedDestination);
        }
    }
    
    bPawnClicked = false;
}

void AMobaDegreePlayerController::ProcessTargetSelection(AActor* TargetActor)
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] ProcessTargetSelection: %s, Current target: %s"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
        *TargetActor->GetName(), 
        PlayerCharacter && PlayerCharacter->AttackTarget ? *PlayerCharacter->AttackTarget->GetName() : TEXT("None"));

    if (PlayerCharacter && PlayerCharacter->AttackTarget == TargetActor)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Already targeting %s"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
            *TargetActor->GetName());
        return;
    }

    AActor* OldTarget = PlayerCharacter ? PlayerCharacter->AttackTarget : nullptr;
    if (OldTarget)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Locally disabling outline for old target: %s"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
            *OldTarget->GetName());
        
        if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OldTarget))
        {
            MobaInteraction->Execute_ShowOutline(OldTarget, false);
        }
    }

    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Locally enabling outline for new target: %s"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
        *TargetActor->GetName());
    
    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(TargetActor))
    {
        MobaInteraction->Execute_ShowOutline(TargetActor, true);
    }

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
    if (!OutlineActor)
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("[%s] ChangeOutline called with null actor"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"));
        return;
    }

    if (!IsLocalController())
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("[%s] ChangeOutline called on non-local controller for %s"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
            *OutlineActor->GetName());
        return;
    }
    
    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] ChangeOutline: Actor=%s, Show=%s"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
        *OutlineActor->GetName(), ShowOutline ? TEXT("true") : TEXT("false"));
        
    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OutlineActor))
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Calling ShowOutline interface on %s"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
            *OutlineActor->GetName());
        MobaInteraction->Execute_ShowOutline(OutlineActor, ShowOutline);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("[%s] Actor %s does not implement IMobaInteraction"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
            *OutlineActor->GetName());
    }
}

void AMobaDegreePlayerController::Server_SelectTarget_Implementation(AActor* Target)
{
    AMobaDegreeCharacter* TargetCharacter = nullptr;
    
    if (PlayerCharacter)
    {
        TargetCharacter = PlayerCharacter;
    }
    else
    {
        TargetCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
    }
    
    if (!IsValid(TargetCharacter) || !IsValid(Target)) 
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("[SERVER] Server_SelectTarget: Invalid Character or Target. PlayerCharacter=%s, GetPawn()=%s, Target=%s"), 
            PlayerCharacter ? *PlayerCharacter->GetName() : TEXT("None"),
            GetPawn() ? *GetPawn()->GetName() : TEXT("None"),
            Target ? *Target->GetName() : TEXT("None"));
        return;
    }
    
    AActor* OldTarget = TargetCharacter->AttackTarget;
    TargetCharacter->OldAttackTarget = OldTarget;
    TargetCharacter->AttackTarget = Target;

    PlayerCharacter = TargetCharacter;

    Client_OnTargetChanged(OldTarget, Target);
}

void AMobaDegreePlayerController::Client_OnTargetChanged_Implementation(AActor* OldTarget, AActor* NewTarget)
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("[%s] Client_OnTargetChanged: Old=%s, New=%s"), 
        IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"), 
        OldTarget ? *OldTarget->GetName() : TEXT("None"), 
        NewTarget ? *NewTarget->GetName() : TEXT("None"));

    if (!IsLocalController())
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("[%s] Client_OnTargetChanged called on non-local controller"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"));
        return;
    }

    if (PlayerCharacter)
    {
        PlayerCharacter->AttackTarget = NewTarget;
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("[%s] Client_OnTargetChanged: PlayerCharacter is null"), 
            IsRunningDedicatedServer() ? TEXT("SERVER") : IsLocalPlayerController() ? TEXT("CLIENT") : TEXT("HOST"));
    }
}

void AMobaDegreePlayerController::Server_ClearTarget_Implementation()
{
    if (!IsValid(PlayerCharacter)) 
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("[SERVER] Server_ClearTarget: Invalid PlayerCharacter"));
        return;
    }
    
    AActor* OldTarget = PlayerCharacter->AttackTarget;
    if (OldTarget)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("[SERVER] Server_ClearTarget: Clearing target %s"), *OldTarget->GetName());
        PlayerCharacter->OldAttackTarget = OldTarget;
        PlayerCharacter->AttackTarget = nullptr;

        Client_OnTargetChanged(OldTarget, nullptr);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("[SERVER] Server_ClearTarget: No target to clear"));
    }
}