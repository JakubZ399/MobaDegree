// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/MinionAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMinionAttributeSet::UMinionAttributeSet()
{
}

void UMinionAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMinionAttributeSet, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMinionAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMinionAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMinionAttributeSet, AttackRange, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMinionAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMinionAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always)
}

void UMinionAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMinionAttributeSet, Health, OldHealth);
}

void UMinionAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMinionAttributeSet, MaxHealth, OldMaxHealth);
}

void UMinionAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMinionAttributeSet, AttackDamage, OldAttackDamage);
}

void UMinionAttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMinionAttributeSet, AttackRange, OldAttackRange);
}

void UMinionAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMinionAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UMinionAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMinionAttributeSet, MovementSpeed, OldMovementSpeed);
}
