// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Interfaces/MinionAI.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MinionBase.generated.h"

class UHealthBarWidget;
class UHealthComponent;
class UBlackboardComponent;
class UWidgetComponent;
class UMobaAttributeSet;
class UTeamComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinionDeath, AMinionBase*, DeadMinion);


UCLASS()
class MOBADEGREE_API AMinionBase : public ACharacter, public IAbilitySystemInterface, public IMobaTeamInterface, public IMobaInteraction, public IMinionAI
{
	GENERATED_BODY()

public:
	AMinionBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS", Instanced)
	TObjectPtr<UMobaAttributeSet> MobaAttributeSet;
	
	UPROPERTY(ReplicatedUsing = OnRep_TeamComponent, VisibleAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS")
	TObjectPtr<UTeamComponent> TeamComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS")
	TSubclassOf<UGameplayEffect> InitEffect;

	//Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|A_GAS")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;
	//
	
	UFUNCTION()
	void OnRep_TeamComponent();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USkeletalMesh> BlueMinionMesh;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USkeletalMesh> RedMinionMesh;

	UFUNCTION(NetMulticast, Reliable)
	void ChangeMesh();

	UFUNCTION(BlueprintCallable)
	void BindOnAttackTarget(class AMinionsGroupPawn* MinionsGroup);

	UFUNCTION(BlueprintCallable)
	void SetupAttackTarget(AActor* AttackTargetRef);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> AttackTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TSubclassOf<UGameplayAbility> BaseAttack;
	
	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void SetGroupPosition(FVector GroupPositionRef);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector GroupPosition;

	UFUNCTION(BlueprintCallable)
	UBlackboardComponent* GetBlackboardComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> HomeBase;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere)
	FOnMinionDeath OnMinionDeath;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere)
	FOnAttackEnd OnAttackEnd;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UParticleSystem> DeathParticle;

	virtual void ShowOutline_Implementation(bool EnableOutline) override;

	UFUNCTION(BlueprintCallable)
	virtual void CallOnAttackEndInterface_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|AI", meta = (AllowPrivateAccess = "true"))
	FName GroupPositionKey{"GroupPosition"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|AI", meta = (AllowPrivateAccess = "true"))
	FName AttackTargetKey{"AttackTarget"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|AI", meta = (AllowPrivateAccess = "true"))
	FName InCombatKey{"InCombat"};

	bool DoOnce{false};

public:

	FORCEINLINE AActor* GetAttackTarget() { return AttackTarget; }
	
	UFUNCTION(BlueprintCallable)
	float GetAttackRange();

	virtual EGameTeam GetTeamInterface_Implementation() const override;

	virtual AMinionBase* GetMinionRef_Implementation() override;

	UFUNCTION(BlueprintCallable)
	virtual float GetAttackRadiusAttribute_Implementation() override;

	virtual void EnemyInfoAI_Implementation(USkeletalMeshComponent* &MeshComponent, AActor*& AttackTargetRef, AMinionBase*& SelfRef) override;
	
};
