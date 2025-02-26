// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaDegreePlayerController.h"
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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMobaDegreePlayerController::AMobaDegreePlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	bPawnClicked = false;
}

void AMobaDegreePlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	PlayerCharacter = Cast<AMobaDegreeCharacter>(GetPawn());
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

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AMobaDegreePlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AMobaDegreePlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AMobaDegreePlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AMobaDegreePlayerController::OnTouchReleased);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMobaDegreePlayerController::OnInputStarted()
{
	StopMovement();
}

void AMobaDegreePlayerController::OnSetDestinationTriggered()
{
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	FHitResult Hit;
	bool bHitSuccessful = false;
	
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	FHitResult HitPawnResult;
	bool bHitSuccessfulHitPawn = GetHitResultUnderCursor(ECC_Pawn, false, HitPawnResult);

	if (bHitSuccessfulHitPawn && PlayerCharacter)
	{
		AActor* HitActor = HitPawnResult.GetActor();
		
		if (!HitActor || HitActor == PlayerCharacter) {return;}

		APawn* HitPawn = Cast<APawn>(HitActor);
		if (HitPawn)
		{
			bPawnClicked = true;
			
			if (PlayerCharacter->AttackTarget == HitActor)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, TEXT("Chose Target"));
			}
			else
			{
				if (PlayerCharacter->AttackTarget)
				{
					PlayerCharacter->OldAttackTarget = PlayerCharacter->AttackTarget;
					ChangeOutline(PlayerCharacter->OldAttackTarget, false);
				}

				PlayerCharacter->AttackTarget = HitActor;
				ChangeOutline(PlayerCharacter->AttackTarget, true);

				FVector WorldDirection = (CachedDestination - PlayerCharacter->GetActorLocation()).GetSafeNormal();
				PlayerCharacter->AddMovementInput(WorldDirection, 1.0, false);
			}

			return;
		}
		else
		{
			if (PlayerCharacter->AttackTarget)
			{
				PlayerCharacter->OldAttackTarget = PlayerCharacter->AttackTarget;
				ChangeOutline(PlayerCharacter->OldAttackTarget, false);
				PlayerCharacter->AttackTarget = nullptr;
			}
		}
		
	}
	else
	{
		bPawnClicked = false;
	}
	
	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AMobaDegreePlayerController::OnSetDestinationReleased()
{
	if (bPawnClicked)
	{
		bPawnClicked = false;
		FollowTime = 0.f;
		return;
	}
	
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AMobaDegreePlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AMobaDegreePlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}
