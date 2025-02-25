// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "MinionBase.generated.h"

class UMinionAttributeSet;

UCLASS()
class MOBADEGREE_API AMinionBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMinionBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A_GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A_GAS")
	TObjectPtr<UMinionAttributeSet> AttributeSet;

private:
	UPROPERTY(EditAnywhere, Category = "A_GAS")
	TSubclassOf<UGameplayEffect> InitEffect;
};
