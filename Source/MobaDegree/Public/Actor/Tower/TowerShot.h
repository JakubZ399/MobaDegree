// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "TowerShot.generated.h"

UCLASS()
class MOBADEGREE_API ATowerShot : public AActor
{
	GENERATED_BODY()
	
public:	
	ATowerShot();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = _GAS)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

protected:

private:



};
