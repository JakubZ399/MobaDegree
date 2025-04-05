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
}

void AMobaDegreePlayerController::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
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
            if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(PlayerCharacter->AttackTarget))
            {
                MobaInteraction->Execute_ShowOutline(PlayerCharacter->AttackTarget, false);
            }
        }

        Server_ClearTarget();
        SpawnCursorFX(CachedDestination);

        if (PlayerCharacter)
        {
            PlayerCharacter->MoveToLocation(CachedDestination);
        }
        else
        {
            PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
        }
    }
    
    bPawnClicked = false;
}

void AMobaDegreePlayerController::ProcessTargetSelection(AActor* TargetActor)
{
    if (PlayerCharacter && PlayerCharacter->AttackTarget == TargetActor)
    {
        return;
    }

    AActor* OldTarget = PlayerCharacter ? PlayerCharacter->AttackTarget : nullptr;
    if (OldTarget)
    {
        if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OldTarget))
        {
            MobaInteraction->Execute_ShowOutline(OldTarget, false);
        }
    }
    
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
    if (!OutlineActor || !IsLocalController())
    {
        return;
    }
    
    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OutlineActor))
    {
        MobaInteraction->Execute_ShowOutline(OutlineActor, ShowOutline);
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
    if (!IsLocalController())
    {
        return;
    }

    if (PlayerCharacter)
    {
        PlayerCharacter->AttackTarget = NewTarget;
    }
}

void AMobaDegreePlayerController::Server_ClearTarget_Implementation()
{
    if (!IsValid(PlayerCharacter)) 
    {
        return;
    }
    
    AActor* OldTarget = PlayerCharacter->AttackTarget;
    if (OldTarget)
    {
        PlayerCharacter->OldAttackTarget = OldTarget;
        PlayerCharacter->AttackTarget = nullptr;

        Client_OnTargetChanged(OldTarget, nullptr);
    }
}