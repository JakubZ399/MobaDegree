// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "TargetDataScreenTrace.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_ScreenTrace : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	// Mniej danych po sieci + zgodne z NetSerialize
	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize Start = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize End   = FVector::ZeroVector;

	FGameplayAbilityTargetData_ScreenTrace() {}
	FGameplayAbilityTargetData_ScreenTrace(const FVector& InStart, const FVector& InEnd)
		: Start(InStart), End(InEnd) {}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	// === KLUCZOWE: własna serializacja sieciowa ===
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Start;
		Ar << End;
		bOutSuccess = true;
		return true;
	}

#if WITH_EDITOR
	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("ScreenTrace: Start=%s End=%s"),
			*Start.ToString(), *End.ToString());
	}
#endif
};

// === Specjalizacja traits, żeby UE wiedziało, że mamy NetSerialize ===
template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_ScreenTrace>
	: public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_ScreenTrace>
{
	enum
	{
		WithNetSerializer = true,   // <- wymagane
		WithCopy         = true,    // opcjonalnie, ale praktyczne
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScreenTraceDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

UCLASS()
class MOBADEGREE_API UTargetDataScreenTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks",
			  meta=(DisplayName="TargetDataScreenTrace",
					HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UTargetDataScreenTrace* CreateTargetDataScreenTrace(UGameplayAbility* OwningAbility, float TraceDistance = 10000.f);

	UPROPERTY(BlueprintAssignable)
	FScreenTraceDataSignature ValidData;

private:
	virtual void Activate() override;

	void SendScreenTraceData(); // client → build Handle(Start/End) → replicate
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);

	UPROPERTY()
	float TraceDistance = 3000.f;
};