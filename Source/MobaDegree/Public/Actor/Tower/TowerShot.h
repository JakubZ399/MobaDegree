// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "TowerShot.generated.h"

class USphereComponent;

UCLASS()
class MOBADEGREE_API ATowerShot : public AActor
{
	GENERATED_BODY()
	
public:	
	ATowerShot();
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = _GAS)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UFUNCTION(BlueprintCallable)
	void AttackTarget(AActor* Target);
	
protected:
	virtual void BeginPlay() override;
private:

	void UpdateAlpha();
	float AlphaToInterpolation{0.f};
	FTimerHandle LerpTimer;

	UFUNCTION()
	void SphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> AttackTargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> TowerShotEffect;

};
