// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "MinionBase.generated.h"

class UMinionAttributeSet;

UCLASS()
class MOBADEGREE_API AMinionBase : public ACharacter
{
	GENERATED_BODY()

public:
	AMinionBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UMinionAttributeSet> AttributeSet;
};
