// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MobaDegreePlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class AMobaDegreeCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class AMobaDegreePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMobaDegreePlayerController();

    /** Time Threshold to know if it was a short press */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    float ShortPressThreshold = 0.2f;

    /** FX Class that we will spawn when clicking */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UNiagaraSystem* FXCursor;

    /** MappingContext */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;
    
    /** Destination Click Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
    UInputAction* SetDestinationClickAction;

    UFUNCTION(BlueprintCallable)
    void ChangeOutline(AActor* OutlineActor, bool ShowOutline);

protected:
    virtual void SetupInputComponent() override;
    
    // To add mapping context
    virtual void BeginPlay() override;

    /** Input handlers for SetDestination action. */
    void OnInputStarted();
    void OnSetDestinationTriggered();
    void OnSetDestinationReleased();

    // Spawn cursor FX locally (client only)
    void SpawnCursorFX(const FVector& Location);
    
    // Handle movement to a point
    void ProcessMovementToLocation(const FVector& Location);
    
    // Handle target selection
    void ProcessTargetSelection(AActor* TargetActor);
    
    // Replication functions
    UFUNCTION(Server, Reliable)
    void Server_MoveToLocation(const FVector& Location);
    
    UFUNCTION(Server, Reliable)
    void Server_SelectTarget(AActor* Target);

    UFUNCTION(Server, Reliable)
    void Server_ClearTarget();

private:
    FVector CachedDestination;
    bool bPawnClicked = false;
    float FollowTime = 0.0f; // For how long it has been pressed

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMobaDegreeCharacter> PlayerCharacter;
};