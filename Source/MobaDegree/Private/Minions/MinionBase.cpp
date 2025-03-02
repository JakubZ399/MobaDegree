// Fill out your copyright notice in the Description page of Project Settings.


#include "Minions/MinionBase.h"
#include "GAS/AttributeSets/MinionAttributeSet.h"

AMinionBase::AMinionBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMinionAttributeSet>(TEXT("AttributeSet"));

}

UAbilitySystemComponent* AMinionBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMinionBase::BeginPlay()
{
	Super::BeginPlay();

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1 , EffectContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void AMinionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


