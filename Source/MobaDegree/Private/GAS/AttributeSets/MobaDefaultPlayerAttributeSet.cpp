// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/MobaDefaultPlayerAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMobaDefaultPlayerAttributeSet::UMobaDefaultPlayerAttributeSet()
	: Health(500.f)
{
}

void UMobaDefaultPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMobaDefaultPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always)
}

void UMobaDefaultPlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaDefaultPlayerAttributeSet, Health, OldHealth);
}
