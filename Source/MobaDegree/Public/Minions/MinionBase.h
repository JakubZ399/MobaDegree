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

class UBlackboardComponent;
class UWidgetComponent;
class UMobaAttributeSet;
class UTeamComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinionDeath);


UCLASS()
class MOBADEGREE_API AMinionBase : public ACharacter, public IAbilitySystemInterface, public IMobaTeamInterface, public IMobaInteraction, public IMinionAI
{
	GENERATED_BODY()

public:
	AMinionBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A_GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A_GAS")
	TObjectPtr<UMobaAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UTeamComponent> TeamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USkeletalMesh> BlueMinionMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USkeletalMesh> RedMinionMesh;

	UFUNCTION(BlueprintCallable)
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

	virtual void CallOnAttackEndInterface_Implementation() override;


	

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "A_GAS")
	TSubclassOf<UGameplayEffect> InitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|AI", meta = (AllowPrivateAccess = "true"))
	FName GroupPositionKey{"GroupPosition"};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup|AI", meta = (AllowPrivateAccess = "true"))
	FName AttackTargetKey{"AttackTarget"};

	bool DoOnce{false};

public:

	FORCEINLINE AActor* GetAttackTarget() { return AttackTarget; }
	
	UFUNCTION(BlueprintCallable)
	float GetAttackRange();

	virtual EGameTeam GetTeamInterface_Implementation() const override;

	virtual AMinionBase* GetMinionRef_Implementation() override;

	virtual float GetAttackRadiusAttribute_Implementation() override;

	virtual void EnemyInfoAI_Implementation(USkeletalMeshComponent* &MeshComponent, AActor*& AttackTargetRef, AMinionBase*& SelfRef) override;
	
};
