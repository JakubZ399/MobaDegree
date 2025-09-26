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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ProjectileSpawner;

	// --- ANIMATION STATE (replicated)
	UPROPERTY(Replicated, BlueprintReadWrite, Category="Tower|Animation")
	bool bAiming = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category="Tower|Animation")
	bool bIsFiring = false;

	UPROPERTY(Replicated, BlueprintReadWrite, Category="Tower|Animation")
	float DistanceToTarget = 0.f;

	UPROPERTY(Replicated, BlueprintReadWrite, Category="Tower|Animation")
	FVector AttackTargetPosition = FVector::ZeroVector;

	// --- AIMING TUNABLES
	UPROPERTY(EditAnywhere, Category="Tower|Aiming")
	float AimYawInterpSpeed = 6.f;

	UPROPERTY(EditAnywhere, Category="Tower|Aiming")
	float AimToleranceDegrees = 3.f;

	UPROPERTY(EditAnywhere, Category="Tower|Aiming")
	float FireAngleToleranceDegrees = 2.f;

	UPROPERTY(BlueprintReadOnly, Category="Tower|Aiming")
	float AlphaRotation = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Tower|Aiming")
	float AlphaRotationSpeed = 0.25;

	UPROPERTY(EditAnywhere, Category="Tower|Aiming")
	bool bAlphaRotation = false;

	void UpdateAiming(float DeltaTime);
	void StartAttackLoop();
	void StopAttackLoop(); 

	UFUNCTION() void ResetIsFiring();
	
	UPROPERTY(BlueprintReadWrite, Category = "Tower|Animation")
	bool bOpenPanel = false;
	
	UPROPERTY(BlueprintReadWrite, Category = "Tower|Animation")
	bool bOpenShield = false;

	UPROPERTY(BlueprintReadWrite, Category = "Tower|Animation")
	bool bIsHit = false;

	UPROPERTY(EditDefaultsOnly, Category = "Tower|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Tower|GAS")
	void ActivateAbilityAttack();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|GAS")
	TSubclassOf<UGameplayAbility> AbilityAttack;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
		void OnTargetEnteredRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
		void OnTargetExitedRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void SpawnTowerShot();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USphereComponent> TowerRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USkeletalMeshComponent> TowerMesh;

	UPROPERTY(EditAnywhere, Category = "Tower|Setup", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UGameplayAbility> TowerAttackClass;
    
	UPROPERTY(EditAnywhere, Category = "Tower|Setup")
		TSubclassOf<ATowerShot> TowerShotClass;

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

	UPROPERTY(EditDefaultsOnly)
		float TowerAttackTime{3.f};
	
public:

	UFUNCTION(BlueprintCallable)
		FORCEINLINE USkeletalMeshComponent* GetTowerMesh() const { return TowerMesh; }

	UFUNCTION(BlueprintCallable)
		float GetAttributeTower(FGameplayAttribute AttributeType);
};


