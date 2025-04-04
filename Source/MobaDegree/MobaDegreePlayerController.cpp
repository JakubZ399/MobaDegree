// Copyright Epic Games, Inc. All Rights Reserved.

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
    // Call the base class  
    Super::BeginPlay();

    // Referencja do PlayerCharacter jest używana tylko lokalnie przez klienta
    if (IsLocalController())
    {
        PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
    }
}

void AMobaDegreePlayerController::SetupInputComponent()
{
    // set up gameplay key bindings
    Super::SetupInputComponent();

    // Add Input Mapping Context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }

    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Setup mouse input events
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
    StopMovement();
}

void AMobaDegreePlayerController::OnSetDestinationTriggered()
{
    FollowTime += GetWorld()->GetDeltaSeconds();
    
    // Sprawdź, czy kliknięto pawna
    FHitResult HitPawnResult;
    bool bHitSuccessfulHitPawn = GetHitResultUnderCursor(ECC_Pawn, false, HitPawnResult);

    if (bHitSuccessfulHitPawn && PlayerCharacter)
    {
        AActor* HitActor = HitPawnResult.GetActor();
        
        if (HitActor && HitActor != PlayerCharacter)
        {
            // Sprawdź czy to pawn i czy to przeciwnik
            APawn* HitPawn = Cast<APawn>(HitActor);
            if (HitPawn)
            {
                // Sprawdź czy jest wrogiem przez interface MobaTeamInterface
                IMobaTeamInterface* TeamInterface = Cast<IMobaTeamInterface>(HitPawn);
                IMobaTeamInterface* PlayerTeamInterface = Cast<IMobaTeamInterface>(PlayerCharacter);
                
                // Jeśli to wróg, to reaguj na kliknięcie
                if (TeamInterface && PlayerTeamInterface && 
                    TeamInterface->Execute_GetTeamInterface(HitPawn) != PlayerTeamInterface->Execute_GetTeamInterface(PlayerCharacter))
                {
                    bPawnClicked = true;
                    ProcessTargetSelection(HitActor);
                    return;
                }
                // Ignoruj kliknięcia na sojusznikach
            }
        }
    }
    
    // Jeśli nie kliknięto pawna lub jest sojusznikiem, spróbuj ruszyć się do lokacji
    FHitResult Hit;
    bool bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
    
    if (bHitSuccessful)
    {
        // Gdy mamy zaznaczony cel, to wyczyść go
        if (PlayerCharacter && PlayerCharacter->AttackTarget)
        {
            Server_ClearTarget();
        }
        
        CachedDestination = Hit.Location;
        ProcessMovementToLocation(CachedDestination);
        
        // Pokaż efekt kursora tylko dla klienta
        SpawnCursorFX(CachedDestination);
    }
}

void AMobaDegreePlayerController::ProcessTargetSelection(AActor* TargetActor)
{
    // Już wybrano ten cel - nie rób nic
    if (PlayerCharacter && PlayerCharacter->AttackTarget == TargetActor)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, TEXT("Already targeting"));
        return;
    }
    
    // Wybierz nowy cel
    Server_SelectTarget(TargetActor);
}

void AMobaDegreePlayerController::ProcessMovementToLocation(const FVector& Location)
{
    // Przekaż komendę ruchu do serwera
    Server_MoveToLocation(Location);
}

void AMobaDegreePlayerController::OnSetDestinationReleased()
{
    if (bPawnClicked)
    {
        bPawnClicked = false;
        FollowTime = 0.f;
        return;
    }
    
    // Jeśli to było krótkie naciśnięcie i nie używamy już żadnych wizualizacji
    // bo SpawnCursorFX wykonuje się w OnSetDestinationTriggered
    
    FollowTime = 0.f;
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

void AMobaDegreePlayerController::Server_MoveToLocation_Implementation(const FVector& Location)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) 
    {
        UE_LOG(LogTemp, Error, TEXT("No controlled pawn found on server!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Moving pawn: %s to location: %s"), 
        *ControlledPawn->GetName(), *Location.ToString());
    
    // Metoda 1: Użyj komponentu poruszania postaci bezpośrednio
    UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(ControlledPawn->GetMovementComponent());
    if (MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Using Character Movement Component"));
        FVector Direction = (Location - ControlledPawn->GetActorLocation()).GetSafeNormal();
        ControlledPawn->AddMovementInput(Direction, 1.0f, true);
        return;
    }
    
    // Metoda 2: Tymczasowo stwórz AIController jeśli nie jest używany
    if (!Cast<AAIController>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating temporary AIController"));
        AAIController* TempAIController = GetWorld()->SpawnActor<AAIController>();
        if (TempAIController)
        {
            TempAIController->Possess(ControlledPawn);
            TempAIController->MoveToLocation(Location, -1.0f, true);
            
            // Zaplanuj powrót do PlayerController
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, [this, TempAIController, ControlledPawn]()
            {
                if (TempAIController && ControlledPawn)
                {
                    TempAIController->UnPossess();
                    Possess(ControlledPawn);
                    TempAIController->Destroy();
                }
            }, 0.1f, false);
            return;
        }
    }
    
    // Metoda 3: Ostatnia deska ratunku - użyj AddActorWorldOffset
    UE_LOG(LogTemp, Warning, TEXT("Using AddActorWorldOffset as last resort"));
    FVector Direction = (Location - ControlledPawn->GetActorLocation()).GetSafeNormal();
    ControlledPawn->SetActorRotation(Direction.Rotation());
    
    // Zaplanuj serię ruchu
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this, ControlledPawn, Direction]()
    {
        if (ControlledPawn)
        {
            ControlledPawn->AddActorWorldOffset(Direction * 10.0f);
        }
    }, 0.01f, true, 0.0f);
}

void AMobaDegreePlayerController::Server_SelectTarget_Implementation(AActor* Target)
{
    if (!IsValid(PlayerCharacter) || !IsValid(Target)) 
    {
        return;
    }
    
    // Wyczyść podświetlenie starego celu
    if (PlayerCharacter->AttackTarget)
    {
        PlayerCharacter->OldAttackTarget = PlayerCharacter->AttackTarget;
        ChangeOutline(PlayerCharacter->OldAttackTarget, false);
    }
    
    // Ustaw nowy cel
    PlayerCharacter->AttackTarget = Target;
    ChangeOutline(PlayerCharacter->AttackTarget, true);
    
    // Opcjonalnie - rusz się w kierunku celu
    UE_LOG(LogTemplateCharacter, Log, TEXT("Target selected: %s"), *Target->GetName());
}

void AMobaDegreePlayerController::Server_ClearTarget_Implementation()
{
    if (!IsValid(PlayerCharacter) || !PlayerCharacter->AttackTarget) 
    {
        return;
    }
    
    PlayerCharacter->OldAttackTarget = PlayerCharacter->AttackTarget;
    ChangeOutline(PlayerCharacter->OldAttackTarget, false);
    PlayerCharacter->AttackTarget = nullptr;
}