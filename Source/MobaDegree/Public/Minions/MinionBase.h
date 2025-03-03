// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MinionBase.generated.h"

class UWidgetComponent;
class UMobaAttributeSet;
class UTeamComponent;

UCLASS()
class MOBADEGREE_API AMinionBase : public ACharacter, public IAbilitySystemInterface, public IMobaTeamInterface
{
	GENERATED_BODY()

public:
	AMinionBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A_GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A_GAS")
	TObjectPtr<UMobaAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UTeamComponent> TeamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	virtual EGameTeam GetTeamInterface_Implementation() const override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "A_GAS")
	TSubclassOf<UGameplayEffect> InitEffect;
};
