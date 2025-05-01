// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MobaBasePawn.h"
#include "MobaTower.generated.h"

class ATowerShot;
class USphereComponent;
class UAttackComponent;
class UGameplayAbility;

UCLASS()
class MOBADEGREE_API AMobaTower : public AMobaBasePawn
{
	GENERATED_BODY()
	
public:    
	AMobaTower();
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ProjectileSpawner;
	
protected:
	virtual void BeginPlay() override;

	void StartAttackSequence();
	
	UFUNCTION()
	void OnTargetEnteredRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnTargetExitedRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void SpawnTowerShot();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> TowerRadius;

	UPROPERTY(EditAnywhere, Category = "Tower|Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> TowerAttackClass;
    
	UPROPERTY(EditAnywhere, Category = "Tower|Setup")
	TSubclassOf<ATowerShot> TowerShotClass;
    
	FTransform ProjectileSpawnerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking{ false };

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> TargetsInRange;

	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void SelectNextTarget();
	
	bool bIsLoaded{false};

	FTimerHandle AttackTimerHandle;

	UFUNCTION()
	void OnAttackTimerComplete();

	UPROPERTY(EditDefaultsOnly)
	float TowerAttackTime{3.f};


public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FTransform GetProjectileSpawnerTransform() const { return ProjectileSpawnerTransform; }

	UFUNCTION(BlueprintCallable)
	float GetAttributeTower(FGameplayAttribute AttributeType);
};


