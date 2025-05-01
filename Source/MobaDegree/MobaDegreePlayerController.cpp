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
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Components/SplineComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMobaDegreePlayerController::AMobaDegreePlayerController()
{
    bReplicates = true;
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
    CachedDestination = FVector::ZeroVector;

    SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline Component");
}

void AMobaDegreePlayerController::AutoRun()
{
    if (!bAutoRunning) return;
    if (PlayerCharacter)
    {
        const FVector LocationOnSpline = SplineComponent->FindLocationClosestToWorldLocation(PlayerCharacter->GetActorLocation(), ESplineCoordinateSpace::World);
        const FVector Direction = SplineComponent->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
        PlayerCharacter->AddMovementInput(Direction);

        const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
        if (DistanceToDestination <= AutoRunAcceptanceRadius)
        {
            bAutoRunning = false;
        }
    }
}



void AMobaDegreePlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AutoRun();
    TraceCursor();
}

void AMobaDegreePlayerController::TraceCursor()
{
    if (!PlayerCharacter)
    {
        return;
    }

    FHitResult HitPawnResult;
    bool bHitSuccessfulHitPawn = GetHitResultUnderCursor(ECC_GameTraceChannel1, true, HitPawnResult);
    AActor* HitActor = bHitSuccessfulHitPawn ? HitPawnResult.GetActor() : nullptr;

    if (bHitSuccessfulHitPawn)
    {
        GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Red, TEXT("bHitSuccessfulHitPawn"));
    }
    if (!bHitSuccessfulHitPawn)
    {
        GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("!bHitSuccessfulHitPawn"));
    }
    
    if (HitActor && HitActor != PlayerCharacter)
    {
        if (HitActor->IsA(APawn::StaticClass()))
        {
            if (HitActor->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()) && HitActor->GetClass()->ImplementsInterface(UMobaInteraction::StaticClass()))
            {
                if (PlayerCharacter->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()) && PlayerCharacter->GetClass()->ImplementsInterface(UMobaInteraction::StaticClass()))
                {
                    HighlightedActor = HitActor;
                    
                    EGameTeam PlayerTeam = IMobaTeamInterface::Execute_GetTeamInterface(PlayerCharacter);
                    EGameTeam TargetTeam = IMobaTeamInterface::Execute_GetTeamInterface(HitActor);

                    IMobaInteraction::Execute_ShowOutline(HighlightedActor, true, 2);

                    if (PlayerTeam != TargetTeam)
                    {
                        EnemyCursorHitActor = HitActor;
                    }
                }
            }
        }
    }
    else if (!HitActor && HighlightedActor)
    {
        IMobaInteraction::Execute_ShowOutline(HighlightedActor, false, 0);
        GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Black, TEXT("No Hit Actor"));
    }
}

void AMobaDegreePlayerController::OnSetDestinationReleased()
{
    /*if (EnemyCursorHitActor)
    {
        GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Emerald, 
    FString::Printf(TEXT("Enemy Target: %s"), *EnemyCursorHitActor->GetName()));
        
        /*bPawnClicked = true;
        ProcessTargetSelection(HighlightedActor);#1#
        return;
    }*/
    
    ClickToMove();
}

void AMobaDegreePlayerController::ClickToMove()
{
    FHitResult Hit;
    bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
    
    if (bHitSuccessful)
    {
        FollowTime += GetWorld()->GetDeltaSeconds();
        CachedDestination = Hit.Location;

        if (PlayerCharacter && PlayerCharacter->AttackTarget)
        {
            if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(PlayerCharacter->AttackTarget))
            {
                MobaInteraction->Execute_ShowOutline(PlayerCharacter->AttackTarget, false, 0);
            }
        }

        Server_ClearTarget();
        SpawnCursorFX(CachedDestination);

        if (PlayerCharacter)
        {
            if (FollowTime <= ShortPressThreshold)
            {
                if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, PlayerCharacter->GetActorLocation(), CachedDestination))
                {
                    SplineComponent->ClearSplinePoints();
                    for (const FVector& Locaction : NavPath->PathPoints)
                    {
                        SplineComponent->AddSplinePoint(Locaction, ESplineCoordinateSpace::World);
                        DrawDebugSphere(GetWorld(), Locaction, 5.f, 8, FColor::Yellow, false, 5.f);
                    }
                    if (NavPath->PathPoints.Num() > 0)
                    {
                        CachedDestination = NavPath->PathPoints.Last();
                    }

                    bAutoRunning = true;
                }
            }
            FollowTime = 0;
            bTargeting = true;
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
            MobaInteraction->Execute_ShowOutline(OldTarget, false, 0);
        }
    }
    
    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(TargetActor))
    {
        MobaInteraction->Execute_ShowOutline(TargetActor, true, 1);
    }

    Server_SelectTarget(TargetActor);
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

void AMobaDegreePlayerController::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());

    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    SetInputMode(InputMode);

    UE_LOG(LogTemp, Error, TEXT("PlayerController::BeginPlay"));
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
        EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::OnSetDestinationTriggered);
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
    //StopMovement();
}

void AMobaDegreePlayerController::OnSetDestinationTriggered()
{
    /*FHitResult Hit;
    bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
    
    if (bHitSuccessful)
    {
        FollowTime += GetWorld()->GetDeltaSeconds();
        CachedDestination = Hit.Location;

        /#1#*if (PlayerCharacter && PlayerCharacter->AttackTarget)
        {
            if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(PlayerCharacter->AttackTarget))
            {
                MobaInteraction->Execute_ShowOutline(PlayerCharacter->AttackTarget, false);
            }
        }#1#

        Server_ClearTarget();
        SpawnCursorFX(CachedDestination);#1#

        if (PlayerCharacter)
        {
            //PlayerCharacter->MoveToLocation(CachedDestination);

            const FVector WorldDirection = CachedDestination - PlayerCharacter->GetActorLocation().GetSafeNormal();
            PlayerCharacter->AddMovementInput(WorldDirection);
        }
        /*else
        {
            PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
        }#1#
    }
    
    //bPawnClicked = false;*/
}


void AMobaDegreePlayerController::SpawnCursorFX(const FVector& Location)
{
    if (FXCursor && IsLocalController())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
    }
}

/*void AMobaDegreePlayerController::ChangeOutline(AActor* OutlineActor, bool ShowOutline)
{
    if (!OutlineActor || !IsLocalController())
    {
        return;
    }
    
    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(OutlineActor))
    {
        MobaInteraction->Execute_ShowOutline(OutlineActor, ShowOutline);
    }
}*/

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