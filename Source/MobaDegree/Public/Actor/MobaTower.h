// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MobaTower.generated.h"

class ATowerShot;
class UHealthBarWidget;
class UWidgetComponent;
class UCapsuleComponent;
class UMobaAttributeSet;
class UTeamComponent;
class UAttackComponent;
class USphereComponent;
class UHealthComponent;
UCLASS()
class MOBADEGREE_API AMobaTower : public APawn, public IAbilitySystemInterface, public IMobaTeamInterface, public IMobaInteraction
{
	GENERATED_BODY()
	
public:	
	AMobaTower();
	virtual void Tick(float DeltaTime) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ProjectileSpawner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Instanced)
	TObjectPtr<UMobaAttributeSet> MobaAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UTeamComponent> TeamComponent;

	virtual void ShowOutline_Implementation(bool EnableOutline) override;

	UPROPERTY(ReplicatedUsing = OnRep_bAttributeInitialized)
	bool bAttributeInitialized = false;

	UFUNCTION()
	void OnRep_bAttributeInitialized();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
 
	
protected:
	void InitializeAttribute();
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnAggroRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnAggroRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void SpawnTowerShot();
	
	virtual EGameTeam GetTeamInterface_Implementation() const override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> TowerRadius;

	//Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	UPROPERTY(EditAnywhere, Category = "Tower|Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Tower|Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> TowerAttackClass;
	//
	
	UPROPERTY(EditAnywhere, Category = "Tower|Setup")
	TSubclassOf<UGameplayEffect> InitEffect;

	UPROPERTY(EditAnywhere, Category = "Tower|Setup")
	TSubclassOf<ATowerShot> TowerShotClass;
	
	UPROPERTY()
	TScriptInterface<class IUIInterface> HealthBarWidgetInterface;

	FTransform ProjectileSpawnerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking{ false };
	
	TArray<AActor*> PawnsArray;

	bool bIsLoaded{false};


public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FTransform GetProjectileSpawnerTransform() const { return ProjectileSpawnerTransform; }

	UFUNCTION(BlueprintCallable)
	float GetAttributeTower(FGameplayAttribute AttributeType);
};
