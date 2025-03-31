// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MinionAI.generated.h"

class AMinionBase;
// This class does not need to be modified.
UINTERFACE()
class UMinionAI : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MOBADEGREE_API IMinionAI
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CallOnAttackEndInterface();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnemyInfoAI(USkeletalMeshComponent* &MeshComponent, AActor* &AttackTargetRef, AActor* &SelfRef);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetAttackRadiusAttribute();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AMinionBase* GetMinionRef();
};
