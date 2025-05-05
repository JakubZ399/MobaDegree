// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MobaDegreePlayerController.generated.h"

class UMobaMainUserWidget;
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

    UFUNCTION(Client, Reliable)
    void CreateMainWidget();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UMobaMainUserWidget> MainUserWidget;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TSubclassOf<UMobaMainUserWidget> MainUserWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UNiagaraSystem* FXCursor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputMappingContext* DefaultMappingContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* SetDestinationClickAction;

    UFUNCTION(Client, Reliable)
    void Client_OnTargetChanged(AActor* OldTarget, AActor* NewTarget);

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;

    void OnInputStarted();
    void OnSetDestinationReleased();

    void SpawnCursorFX(const FVector& Location);
    
    UFUNCTION(Server, Reliable)
    void Server_SelectTarget(AActor* Target);
    
    UFUNCTION(Server, Reliable)
    void Server_ClearTarget();
    
    UFUNCTION(Server, Reliable)
    void Server_MoveToLocation(const FVector& Location);

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USplineComponent> SplineComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMobaDegreeCharacter> PlayerCharacter;

    UPROPERTY()
    TObjectPtr<AActor> HoveredActor;

    UPROPERTY()
    TObjectPtr<AActor> AttackTarget;
    
    UPROPERTY()
    float LastTargetChangeTime = 0.0f;
    
    UPROPERTY(EditAnywhere, Category = "Targeting")
    float TargetDebounceTime = 0.1f;
    
    UPROPERTY(EditDefaultsOnly)
    float AutoRunAcceptanceRadius = 50.f;
    
    bool IsEnemyHovered();
    void HandleMovement();
    void AutoRun();
    void TraceCursor();
    void ProcessInput();
    void PerformMovementToLocation(const FVector& Location);
    void PerformTargetSelection(AActor* TargetActor);
};