// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MobaDegreePlayerController.generated.h"

class USplineComponent;
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
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnRep_Pawn() override;

    void AutoRun();
    void TraceCursor();
    void ClickToMove();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UNiagaraSystem* FXCursor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
    UInputAction* SetDestinationClickAction;

    /*UFUNCTION(BlueprintCallable)
    void ChangeOutline(AActor* OutlineActor, bool ShowOutline);*/

    UFUNCTION(Client, Reliable)
    void Client_OnTargetChanged(AActor* OldTarget, AActor* NewTarget);

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;

    void OnInputStarted();
    void OnSetDestinationTriggered();
    void OnSetDestinationReleased();

    void SpawnCursorFX(const FVector& Location);
    void ProcessTargetSelection(AActor* TargetActor);
    
    UFUNCTION(Server, Reliable)
    void Server_SelectTarget(AActor* Target);
    
    UFUNCTION(Server, Reliable)
    void Server_ClearTarget();

private:
    FVector CachedDestination = FVector::ZeroVector;
    float FollowTime = 0;
    float ShortPressThreshold = 0.5f;
    bool bAutoRunning = false;
    bool bTargeting = false;

    UPROPERTY(EditDefaultsOnly)
    float AutoRunAcceptanceRadius = 50.f;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USplineComponent> SplineComponent;
    
    bool bPawnClicked = false;
    float StartClickTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMobaDegreeCharacter> PlayerCharacter;

    UPROPERTY()
    TObjectPtr<AActor> HoveredActor;

    UPROPERTY()
    TObjectPtr<AActor> AttackTarget;
    
    bool IsEnemyHovered();
};