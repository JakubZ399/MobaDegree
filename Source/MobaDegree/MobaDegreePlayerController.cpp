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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMobaDegreePlayerController::AMobaDegreePlayerController()
{
    bReplicates = true;
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;

    SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline Component");
}

void AMobaDegreePlayerController::BeginPlay()
{
    Super::BeginPlay();

    PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());

    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    SplineComponent->ClearSplinePoints();

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
    
    HandleMovement();
    TraceCursor();
}

void AMobaDegreePlayerController::HandleMovement()
{
    if (!PlayerCharacter) return;
    
    if (PlayerCharacter->AttackTarget)
    {
        return;
    }
    
    if (SplineComponent->GetNumberOfSplinePoints() > 0)
    {
        AutoRun();
    }
}

void AMobaDegreePlayerController::TraceCursor()
{
    if (!PlayerCharacter) return;

    FHitResult HitPawnResult;
    bool bHitSuccessfulHitPawn = GetHitResultUnderCursor(ECC_GameTraceChannel1, true, HitPawnResult);
    AActor* CurrentHitActor = nullptr;
    
    if (bHitSuccessfulHitPawn && IsValid(HitPawnResult.GetActor()) && HitPawnResult.GetActor() != PlayerCharacter)
    {
        CurrentHitActor = HitPawnResult.GetActor();
    }

    if (CurrentHitActor != HoveredActor)
    {
        if (IsValid(HoveredActor) && HoveredActor != AttackTarget)
        {
            IMobaInteraction::Execute_ShowOutline(HoveredActor, false, 0);
        }
        
        HoveredActor = CurrentHitActor;

        if (IsValid(HoveredActor) && HoveredActor != AttackTarget && HoveredActor->GetClass()->ImplementsInterface(UMobaInteraction::StaticClass()))
        {
            if (IsEnemyHovered())
            {
                IMobaInteraction::Execute_ShowOutline(HoveredActor, true, 2);
            }
            else
            {
                IMobaInteraction::Execute_ShowOutline(HoveredActor, true, 3);
            }
        }
    }

    if (!IsValid(CurrentHitActor))
    {
        if (IsValid(HoveredActor) && HoveredActor != AttackTarget)
        {
            IMobaInteraction::Execute_ShowOutline(HoveredActor, false, 0);
        }
        HoveredActor = nullptr;
    }
}

bool AMobaDegreePlayerController::IsEnemyHovered()
{
    if (!IsValid(HoveredActor) || !IsValid(PlayerCharacter))
        return false;
        
    if (!HoveredActor->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()) ||
        !PlayerCharacter->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()))
        return false;
        
    EGameTeam PlayerTeam = IMobaTeamInterface::Execute_GetTeamInterface(PlayerCharacter);
    EGameTeam TargetTeam = IMobaTeamInterface::Execute_GetTeamInterface(HoveredActor);
    
    return PlayerTeam != TargetTeam;
}

void AMobaDegreePlayerController::AutoRun()
{
    if (!PlayerCharacter || SplineComponent->GetNumberOfSplinePoints() == 0) return;
    
    const FVector ActorLocation = PlayerCharacter->GetActorLocation();
    const FVector LocationOnSpline = SplineComponent->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
    const FVector Direction = SplineComponent->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
    
    PlayerCharacter->AddMovementInput(Direction);

    const FVector LastPoint = SplineComponent->GetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
    const float DistanceToDestination = (LocationOnSpline - LastPoint).Length();
    
    if (DistanceToDestination <= AutoRunAcceptanceRadius)
    {
        SplineComponent->ClearSplinePoints();
    }
}

void AMobaDegreePlayerController::OnInputStarted()
{
    bShowMouseCursor = true;  
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
}

void AMobaDegreePlayerController::OnSetDestinationReleased()
{
    ProcessInput();
}

void AMobaDegreePlayerController::ProcessInput()
{
    if (!PlayerCharacter) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (IsValid(HoveredActor) && CurrentTime - LastTargetChangeTime < TargetDebounceTime)
    {
        return;
    }

    if (IsValid(HoveredActor) && IsEnemyHovered())
    {
        PerformTargetSelection(HoveredActor);
        return;
    }

    FHitResult Hit;
    bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
    
    if (bHitSuccessful)
    {
        PerformMovementToLocation(Hit.Location);
    }
}

void AMobaDegreePlayerController::PerformTargetSelection(AActor* TargetActor)
{
    if (!TargetActor || (PlayerCharacter && PlayerCharacter->AttackTarget == TargetActor))
    {
        return;
    }

    AttackTarget = TargetActor;
    LastTargetChangeTime = GetWorld()->GetTimeSeconds();

    if (PlayerCharacter && PlayerCharacter->AttackTarget)
    {
        if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(PlayerCharacter->AttackTarget))
        {
            MobaInteraction->Execute_ShowOutline(PlayerCharacter->AttackTarget, false, 0);
        }
    }

    if (IMobaInteraction* MobaInteraction = Cast<IMobaInteraction>(TargetActor))
    {
        MobaInteraction->Execute_ShowOutline(TargetActor, true, 1);
    }

    Server_SelectTarget(TargetActor);
}

void AMobaDegreePlayerController::PerformMovementToLocation(const FVector& Location)
{
    if (!PlayerCharacter) return;

    if (PlayerCharacter->AttackTarget)
    {
        PlayerCharacter->InterruptCombat();
    }
    
    SpawnCursorFX(Location);
    Server_MoveToLocation(Location);

    if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, PlayerCharacter->GetActorLocation(), Location))
    {
        SplineComponent->ClearSplinePoints();
        for (const FVector& PathPoint : NavPath->PathPoints)
        {
            SplineComponent->AddSplinePoint(PathPoint, ESplineCoordinateSpace::World);
        }
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
}

void AMobaDegreePlayerController::SpawnCursorFX(const FVector& Location)
{
    if (FXCursor && IsLocalController())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Location, FRotator::ZeroRotator, FVector(1.2f, 1.2f, 1.2f), true, true, ENCPoolMethod::None, true);
    }
}

void AMobaDegreePlayerController::Server_SelectTarget_Implementation(AActor* Target)
{
    if (!IsValid(PlayerCharacter) || !IsValid(Target)) return;
    
    PlayerCharacter->SetAttackTarget(Target);
    Client_OnTargetChanged(nullptr, Target);
}

void AMobaDegreePlayerController::Client_OnTargetChanged_Implementation(AActor* OldTarget, AActor* NewTarget)
{
    if (!IsLocalController() || !PlayerCharacter) return;
    
    PlayerCharacter->AttackTarget = NewTarget;
}

void AMobaDegreePlayerController::Server_ClearTarget_Implementation()
{
    if (!IsValid(PlayerCharacter)) return;
    
    PlayerCharacter->ClearAttackTarget();
    Client_OnTargetChanged(nullptr, nullptr);
}

void AMobaDegreePlayerController::Server_MoveToLocation_Implementation(const FVector& Location)
{
    if (!IsValid(PlayerCharacter)) return;
}

void AMobaDegreePlayerController::CreateMainWidget_Implementation()
{
    if (MainUserWidgetClass)
    {
        MainUserWidget = CreateWidget<UMobaMainUserWidget>(this, MainUserWidgetClass);
        MainUserWidget->AddToViewport();
    }
}