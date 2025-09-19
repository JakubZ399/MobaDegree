// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MobaAttributeSet.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MOBADEGREE_API UMobaAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMobaAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, Health)

	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, MaxHealth)

	UPROPERTY(ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, Mana)

	UPROPERTY(ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, MaxMana)
	
	UPROPERTY(ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AttackDamage)

	UPROPERTY(ReplicatedUsing = OnRep_AttackRange)
	FGameplayAttributeData AttackRange;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AttackRange)

	UPROPERTY(ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AttackSpeed)

	UPROPERTY(ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, MovementSpeed)

	UPROPERTY(ReplicatedUsing = OnRep_LifeSteal)
	FGameplayAttributeData LifeSteal;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, LifeSteal)

	//
	UPROPERTY(ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, DamageReduction)

	UPROPERTY(ReplicatedUsing = OnRep_CritChance)
	FGameplayAttributeData CritChance;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, CritChance)

	UPROPERTY(ReplicatedUsing = OnRep_CritMultiplier)
	FGameplayAttributeData CritMultiplier;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, CritMultiplier)

private:
	bool bIsHealthInitialized{false};
	bool bIsManaInitialized{false};

	UFUNCTION(BlueprintCallable)
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage);

	UFUNCTION()
	void OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	UFUNCTION()
	void OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal);

	//
	UFUNCTION()
	void OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction);

	UFUNCTION()
	void OnRep_CritChance(const FGameplayAttributeData& OldCritChance);

	UFUNCTION()
	void OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier);

public:
	//AbilityRMB
	UPROPERTY(ReplicatedUsing = OnRep_AbilityRMBManaCost)
	FGameplayAttributeData AbilityRMBManaCost;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityRMBManaCost)

	UPROPERTY(ReplicatedUsing = OnRep_AbilityRMBCooldown)
	FGameplayAttributeData AbilityRMBCooldown;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityRMBCooldown)

	//AbilityQ
	UPROPERTY(ReplicatedUsing = OnRep_AbilityQManaCost)
	FGameplayAttributeData AbilityQManaCost;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityQManaCost)

	UPROPERTY(ReplicatedUsing = OnRep_AbilityQCooldown)
	FGameplayAttributeData AbilityQCooldown;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityQCooldown)

	//AbilityE
	UPROPERTY(ReplicatedUsing = OnRep_AbilityEManaCost)
	FGameplayAttributeData AbilityEManaCost;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityEManaCost)

	UPROPERTY(ReplicatedUsing = OnRep_AbilityECooldown)
	FGameplayAttributeData AbilityECooldown;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityECooldown)

	//AbilityR
	UPROPERTY(ReplicatedUsing = OnRep_AbilityRManaCost)
	FGameplayAttributeData AbilityRManaCost;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityRManaCost)

	UPROPERTY(ReplicatedUsing = OnRep_AbilityRCooldown)
	FGameplayAttributeData AbilityRCooldown;
	ATTRIBUTE_ACCESSORS(UMobaAttributeSet, AbilityRCooldown)

private:
	//AbilityRMB
	UFUNCTION()
	void OnRep_AbilityRMBManaCost(const FGameplayAttributeData& OldAbilityRMBManaCost);

	UFUNCTION()
	void OnRep_AbilityRMBCooldown(const FGameplayAttributeData& OldAbilityRMBCooldown);

	//AbilityQ
	UFUNCTION()
	void OnRep_AbilityQManaCost(const FGameplayAttributeData& OldAbilityQManaCost);

	UFUNCTION()
	void OnRep_AbilityQCooldown(const FGameplayAttributeData& OldAbilityQCooldown);
	
	//AbilityE
	UFUNCTION()
	void OnRep_AbilityEManaCost(const FGameplayAttributeData& OldAbilityEManaCost);

	UFUNCTION()
	void OnRep_AbilityECooldown(const FGameplayAttributeData& OldAbilityECooldown);

	//AbilityR
	UFUNCTION()
	void OnRep_AbilityRManaCost(const FGameplayAttributeData& OldAbilityRManaCost);

	UFUNCTION()
	void OnRep_AbilityRCooldown(const FGameplayAttributeData& OldAbilityRCooldown);
};

