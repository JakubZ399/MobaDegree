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
    UPROPERTY(Replicated ,VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> AttackTarget;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> OldAttackTarget;
    
    UPROPERTY(ReplicatedUsing = OnRep_CombatState, BlueprintReadOnly)
    ECharacterCombatState CombatState = ECharacterCombatState::Idle;
    
    UFUNCTION()
    void OnRep_CombatState();
    
    // Combat Functions
    UFUNCTION(BlueprintCallable)
    void SetAttackTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable)
    void ClearAttackTarget();
    
    UFUNCTION(BlueprintCallable)
    bool IsInAttackRange() const;
    
    UFUNCTION(BlueprintCallable)
    void InterruptCombat();
    
    UFUNCTION(BlueprintCallable)
    bool CanPerformAction() const;

    UFUNCTION(BlueprintCallable)
    bool IsValidAttackTarget(AActor* Target) const;
    
    // Interface functions
    virtual void ShowOutline_Implementation(bool EnableOutline, int32 OutlineColor) override;
    
    // Gameplay Ability
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Attack")
    TSubclassOf<UGameplayAbility> PrimaryAttackAbility;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual EGameTeam GetTeamInterface_Implementation() const override;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    void InitializeAttribute();
    void InitAbilityActorInfo();
    
    // Combat Logic
    void HandleCombat(float DeltaTime);
    void MoveToTarget();
    void PerformAttack();
    void UpdateCombatState();
    void RotateToTarget(float DeltaTime);
    
    UFUNCTION(Server, Reliable)
    void Server_SetCombatState(ECharacterCombatState NewState);
    
    UFUNCTION(Server, Reliable)
    void Server_PerformAttack();
private:
    UPROPERTY()
    TObjectPtr<AMobaPlayerState> MobaPlayerState;
    
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
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMobaAbilitySystemComponent> AbilitySystemComponent;
    
    UPROPERTY()
    TObjectPtr<UMobaAttributeSet> AttributeSet;
    
    // Combat Properties
    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationSpeed = 10.0f;
    
    float CurrentAttackRange = 0.0f;

public:
    FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    UFUNCTION(BlueprintCallable) FORCEINLINE void SetCombatState(ECharacterCombatState NewState) { CombatState = NewState; }
};