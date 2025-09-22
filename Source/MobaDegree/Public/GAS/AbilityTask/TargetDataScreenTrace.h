// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataScreenTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScreenTraceDataSignature, const FVector&, StartTracePosition, const FVector&, EndTracePosition);

/**
 * 
 */
UCLASS()
class MOBADEGREE_API UTargetDataScreenTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (DisplayName = "GetTargetDataFromPlayerScreenTrace", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataScreenTrace* GetTargetDataFromScreenTrace(UGameplayAbility* OwningAbility, float TraceDistance = 10000.0f);

	UPROPERTY(BlueprintAssignable)
	FScreenTraceDataSignature ValidData;

private:

	virtual void Activate() override;
	void SendDataFromScreenTrace();

	UPROPERTY()
	float TraceDistance;
};
