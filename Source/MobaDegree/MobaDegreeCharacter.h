// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MobaDegreeCharacter.generated.h"

class UMobaAbilitySystemComponent;
class AMobaPlayerState;
class UWidgetComponent;
class UHealthComponent;
class UMobaAttributeSet;

UENUM(BlueprintType)
enum class ECharacterCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Moving      UMETA(DisplayName = "Moving"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Casting     UMETA(DisplayName = "Casting")
};

UCLASS(Blueprintable)
class AMobaDegreeCharacter : public ACharacter, public IAbilitySystemInterface, public IMobaTeamInterface, public IMobaInteraction
{
    GENERATED_BODY()

public:
    AMobaDegreeCharacter();
    virtual void Tick(float DeltaSeconds) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;
    
    // Attack System
    UPROPERTY(Replicated,VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> AttackTarget;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> OldAttackTarget;
    
    // Interface functions
    virtual void ShowOutline_Implementation(bool EnableOutline, int32 OutlineColor) override;
    
    // Gameplay Ability
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> PrimaryAttackAbility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> Ability1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> AbilityRMBClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> AbilityQClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> AbilityEClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> AbilityRClass;

    /***
     * Ability Activation
     */
    UFUNCTION()
    void ActivateAttackAbility();

    UFUNCTION()
    void ActivateRMBAbility();

    UFUNCTION()
    void ActivateQAbility();

    UFUNCTION()
    void ActivateEAbility();

    UFUNCTION()
    void ActivateRAbility();
    /***
    * Ability Activation END
    */

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual EGameTeam GetTeamInterface_Implementation() const override;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    void InitializeAttribute();
    void InitAbilityActorInfo();

private:
    UPROPERTY(BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMobaPlayerState> MobaPlayerState;

    UPROPERTY(BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class AMobaDegreePlayerController> MobaPlayerController;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* TopDownCameraComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHealthComponent> HealthComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UWidgetComponent> HealthBarWidget;
    
    UPROPERTY(EditAnywhere, Category = "Setup")
    TSubclassOf<UUserWidget> HealthBarWidgetClass;
    
    UPROPERTY(EditAnywhere, Category = "Setup|GAS")
    TSubclassOf<UGameplayEffect> InitEffect;
    
    UPROPERTY(EditAnywhere, Category = "Setup|GAS")
    TSubclassOf<UGameplayEffect> HealthRegenEffect;

    UPROPERTY(EditAnywhere, Category = "Setup|GAS")
    TSubclassOf<UGameplayEffect> AbilityInfoEffect;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMobaAbilitySystemComponent> AbilitySystemComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMobaAttributeSet> AttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 900.f;
    
    float CurrentAttackRange = 0.0f;

public:
    FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};