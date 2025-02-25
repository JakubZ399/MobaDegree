// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Team/EGameTeam.h"
#include "MobaTower.generated.h"

class UCapsuleComponent;
class UMinionAttributeSet;
class UTeamComponent;
class UAttackComponent;
class USphereComponent;

UCLASS()
class MOBADEGREE_API AMobaTower : public APawn, public IAbilitySystemInterface
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMinionAttributeSet> AttributeSet;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnAggroRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnAggroRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnTowerShot();
	
private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> TowerRadius;

	FTransform ProjectileSpawnerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking{ false };

	UPROPERTY(EditAnywhere, Category = "A_GAS")
	TSubclassOf<UGameplayEffect> InitEffect;

	TArray<AActor*> PawnsArray;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FTransform GetProjectileSpawnerTransform() const { return ProjectileSpawnerTransform; }
};
