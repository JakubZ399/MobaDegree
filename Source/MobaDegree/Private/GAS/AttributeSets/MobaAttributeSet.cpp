// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/MobaAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMobaAttributeSet::UMobaAttributeSet()
	:  Health(250.f), MaxHealth(250.f), Mana(450.f), MaxMana(450.f), AttackDamage(25.f), AttackRange(30.f), AttackSpeed(1.f), MovementSpeed(300.f)
{
}

void UMobaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
}

void UMobaAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		bIsHealthInitialized = true;
	}
    if (Data.EvaluatedData.Attribute == GetHealthAttribute() && bIsHealthInitialized)
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }

	if (Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		bIsManaInitialized = true;
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute() && bIsManaInitialized)
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
}

void UMobaAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, Mana, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, MaxMana, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AttackRange, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, LifeSteal, COND_None, REPNOTIFY_Always)

	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityRMBManaCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityRMBCooldown, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityQManaCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityQCooldown, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityEManaCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityECooldown, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityRManaCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMobaAttributeSet, AbilityRCooldown, COND_None, REPNOTIFY_Always);
}

void UMobaAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, Health, OldHealth);
}

void UMobaAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, MaxHealth, OldMaxHealth);
}

void UMobaAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, Mana, OldMana);
}

void UMobaAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, MaxMana, OldMaxMana);
}

void UMobaAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AttackDamage, OldAttackDamage);
}

void UMobaAttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AttackRange, OldAttackRange);
}

void UMobaAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UMobaAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, MovementSpeed, OldMovementSpeed);
}

void UMobaAttributeSet::OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, LifeSteal, OldLifeSteal);
}

void UMobaAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, DamageReduction, OldDamageReduction);
}

void UMobaAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldCritChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, CritChance, OldCritChance);
}

void UMobaAttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, CritMultiplier, OldCritMultiplier);
}

void UMobaAttributeSet::OnRep_AbilityRMBManaCost(const FGameplayAttributeData& OldAbilityRMBManaCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityRMBManaCost, OldAbilityRMBManaCost);
}

void UMobaAttributeSet::OnRep_AbilityRMBCooldown(const FGameplayAttributeData& OldAbilityRMBCooldown)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityRMBCooldown, OldAbilityRMBCooldown);
}

void UMobaAttributeSet::OnRep_AbilityQManaCost(const FGameplayAttributeData& OldAbilityQManaCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityQCooldown, OldAbilityQManaCost);
}

void UMobaAttributeSet::OnRep_AbilityQCooldown(const FGameplayAttributeData& OldAbilityQCooldown)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityQCooldown, OldAbilityQCooldown);
}

void UMobaAttributeSet::OnRep_AbilityEManaCost(const FGameplayAttributeData& OldAbilityEManaCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityECooldown, OldAbilityEManaCost);
}

void UMobaAttributeSet::OnRep_AbilityECooldown(const FGameplayAttributeData& OldAbilityECooldown)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityECooldown, OldAbilityECooldown);
}

void UMobaAttributeSet::OnRep_AbilityRManaCost(const FGameplayAttributeData& OldAbilityRManaCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityRCooldown, OldAbilityRManaCost);
}

void UMobaAttributeSet::OnRep_AbilityRCooldown(const FGameplayAttributeData& OldAbilityRCooldown)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaAttributeSet, AbilityRCooldown, OldAbilityRCooldown);
}

