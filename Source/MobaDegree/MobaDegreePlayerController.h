// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MobaDegreePlayerController.generated.h"

class UMobaMainUserWidget;
class USplineComponent;
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class AMobaDegreeCharacter;

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

    UFUNCTION(BlueprintImplementableEvent)
    void InitializeMainWidgetBlueprintVariable();

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* AttackAction;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TObjectPtr<AActor> TraceTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TObjectPtr<AActor> TraceTargetEnemy;

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    FHitResult PerformTraceFromScreen();

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;

    void Look(const FInputActionValue& Value);
    void Move(const FInputActionValue& Value);
    void PerformAttack(const FInputActionValue& Value);

    UFUNCTION(BlueprintImplementableEvent)
    void GetTraceTarget();

    UFUNCTION(BlueprintCallable)
    FHitResult TraceFromeScreenCenter(float Distance);

private:
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMobaDegreeCharacter> PlayerCharacter;

    UPROPERTY()
    TObjectPtr<AActor> HoveredActor;

    UPROPERTY()
    TObjectPtr<AActor> AttackTarget;

public:
    FORCEINLINE AActor* GetAttackTarget() { return AttackTarget; }
};