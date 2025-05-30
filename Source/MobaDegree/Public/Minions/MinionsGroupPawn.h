// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actor/MobaTower.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/MobaTeamInterface.h"
#include "Team/EGameTeam.h"
#include "MinionsGroupPawn.generated.h"

UENUM(BlueprintType)
enum class ETargetTypePriority : uint8
{
	None = 0,
	Player = 1,
	MinionGroup = 2,
	Tower = 3,
	AggressivePlayer = 4
};

class USphereComponent;
enum class EGameTeam : uint8;
class UPawnSensingComponent;
class UFloatingPawnMovement;
class AMinionBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackTargetSet, AActor*, AttackTarget);

UCLASS()
class MOBADEGREE_API AMinionsGroupPawn : public APawn, public IMobaTeamInterface
{
	GENERATED_BODY()

public:
	AMinionsGroupPawn();
	void SetAttackTargetBlackboard(AActor* Target, FName Key);

#pragma region Combat

	UFUNCTION()
	TArray<AActor*> FindActorsInRange();
	
	UFUNCTION()
	void FindValidAttackTarget();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	TArray<AActor*> TargetsInRange;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	bool bInCombat{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	float ScanTime{0.5f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	float ScanRadius{700.f};

	FTimerHandle FindActorTimerHandle;
	
	TMap<ETargetTypePriority, TArray<AActor*>> CategorizedTargets;

	void SelectTargetByPriority();

	void HandleSelectedTarget(AActor* Target, ETargetTypePriority Priority);

	AActor* FindClosestTarget(const TArray<AActor*>& Targets);

	AMinionBase* FindClosestMinionFromGroup(AActor* EnemyGroup);
	
#pragma endregion

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackTargetSet OnAttackTargetSet;

	UFUNCTION(BlueprintCallable)
	void Initialize();
	
	//To setup
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<AActor> EnemyLaneTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<USceneComponent*> SpawnPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	TObjectPtr<APawn> AttackTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	TArray<AMinionBase*> SpawnedMinions;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setup")
	EGameTeam Team{EGameTeam::None};

#pragma region Debug;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Debug")
	bool bDebugRespawnMelee{true};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Debug")
	bool bDebugRespawnRanged{true};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Debug")
	bool bDetectTower{true};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Debug")
	bool bDetectMinion{true};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Debug")
	bool bDetectPlayers{true};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Debug")
	bool bDebugMode{false};
#pragma	endregion

	UFUNCTION(BlueprintCallable)
	void MinionsSpawn(TSubclassOf<AMinionBase> MinionClass, USceneComponent* SpawnPointSceneComponent);

	UFUNCTION(BlueprintCallable)
	void OnMinionDeath(AActor* DeadMinion);

	UFUNCTION(BlueprintCallable)
	bool SetupDetectedEnemy(bool bEnemyBoolDetection, AActor* Actor);
	
	UFUNCTION(BlueprintCallable) void OnEnemyDestroyed(AActor* DestroyedActor);
	
	UFUNCTION(BlueprintCallable) void BindEnemyDestroy(AActor* ActorToBind);
	UFUNCTION(BlueprintCallable) void UnBindBindEnemyDestroy(AActor* ActorToUnBind);


#pragma region MinionsSpawnPoints

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPointsMinion;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPointMinionMelee;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPointMinionRanged;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Melee_Middle;*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Melee_Right;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Melee_Left;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpawnPoints")
	TObjectPtr<USceneComponent> SpawnPoint_Ranged_Middle;*/
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	FName InCombatKey{"InCombat"};

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetEnemyLaneTarget(AActor* Target) { EnemyLaneTarget = Target; }

	UFUNCTION(BlueprintCallable)
	void CallOnAttackTargetSet();

	virtual EGameTeam GetTeamInterface_Implementation() const override;

};
