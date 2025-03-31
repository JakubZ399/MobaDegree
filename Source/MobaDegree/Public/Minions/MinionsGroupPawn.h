// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Team/EGameTeam.h"
#include "MinionsGroupPawn.generated.h"

enum class EGameTeam : uint8;
class UPawnSensingComponent;
class UFloatingPawnMovement;
class AMinionBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackTargetSet, AActor*, AttackTarget);

UCLASS()
class MOBADEGREE_API AMinionsGroupPawn : public APawn
{
	GENERATED_BODY()

public:
	AMinionsGroupPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SetAttackTargetBlackboard(AActor* Target, FName Key);

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackTargetSet OnAttackTargetSet;

	UFUNCTION(BlueprintCallable)
	void Initialize();

	//To setup
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<AActor> EnemyLaneTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<USceneComponent*> SpawnPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	TObjectPtr<APawn> AttackTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	TArray<AMinionBase*> SpawnedMinions;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	EGameTeam Team{EGameTeam::None};

	UFUNCTION(BlueprintCallable)
	void MinionsSpawn(TSubclassOf<AMinionBase> MinionClass, USceneComponent* SpawnPointSceneComponent);

	UFUNCTION(BlueprintCallable)
	void OnMinionDeath();

	UFUNCTION(BlueprintCallable)
	void OnSeePawn(APawn* Pawn);
	
#pragma region MinionsSpawnPoints

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPointsMinion;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPointMinionMelee;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPointMinionRanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Melee_Middle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Melee_Right;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Melee_Left;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Ranged_Middle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Ranged_Right;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Ranged_Left;
	
#pragma endregion

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Setup")
	TSubclassOf<AMinionBase> MeleeMinionClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Setup")
	TSubclassOf<AMinionBase> RangedMinionClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	FName EnemyLaneTargetKey{"EnemyLaneTarget"};

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetEnemyLaneTarget(AActor* Target) { EnemyLaneTarget = Target; }

};
