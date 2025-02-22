// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Team/EGameTeam.h"
#include "MobaTower.generated.h"

class UTeamComponent;
class UAttackComponent;
class USphereComponent;

UCLASS()
class MOBADEGREE_API AMobaTower : public AActor
{
	GENERATED_BODY()
	
public:	
	AMobaTower();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTeamComponent> TeamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttackComponent> AttackComponent;
	
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
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> TowerRadius;




	
};
