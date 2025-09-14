// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MobaDegreePlayerController.generated.h"

class UAbilitySystemComponent;
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* AbilityRMBAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* AbilityQAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* AbilityEAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
    UInputAction* AbilityRAction;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TObjectPtr<AActor> TraceTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TObjectPtr<AActor> TraceTargetEnemy;

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    FHitResult PerformTraceFromScreen();

    //Call Ability UI Cooldown
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void CallAttackAbilityUICooldown();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void CallRMBAbilityUICooldown();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void CallQAbilityUICooldown();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void CallEAbilityUICooldown();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void CallRAbilityUICooldown();
    // \Call Ability UI Cooldown

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;

    void Look(const FInputActionValue& Value);
    void Move(const FInputActionValue& Value);

    UFUNCTION(BlueprintImplementableEvent)
    void GetTraceTarget();

    UFUNCTION(BlueprintCallable)
    FHitResult TraceFromeScreenCenter(float Distance);

private:

    UPROPERTY(BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    
    void PerformAttack(const FInputActionValue& Value);
    void ActivateRMBAbilityCallback(const FInputActionValue& Value);
    void ActivateQAbilityCallback(const FInputActionValue& Value);
    void ActivateEAbilityCallback(const FInputActionValue& Value);
    void ActivateRAbilityCallback(const FInputActionValue& Value);
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMobaDegreeCharacter> PlayerCharacter;

    UPROPERTY()
    TObjectPtr<AActor> HoveredActor;

    UPROPERTY()
    TObjectPtr<AActor> AttackTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRadius = 900.f;

public:
    FORCEINLINE AActor* GetAttackTarget() { return AttackTarget; }
    FORCEINLINE void SetAbilitySystemComponent(UAbilitySystemComponent* NewAbilitySystemComponent) { NewAbilitySystemComponent ? AbilitySystemComponent = NewAbilitySystemComponent : AbilitySystemComponent = nullptr; }
    FORCEINLINE void SetAttackRadius(float NewAttackRadius) { AttackRadius = NewAttackRadius; }
    
};