// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Components/WidgetComponent.h"
#include "HealthComponent.generated.h"

class UHealthBarWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBADEGREE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	UHealthComponent();

	/**
	 * Use this in owner's BeginPlay() to setup HealthBarWidget
	 * @param Widget - HealthBarWidget
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthBarWidgetFromOwner(UWidgetComponent* Widget);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UWidgetComponent> OwnerHealthBar;

	UPROPERTY()
	TObjectPtr<UHealthBarWidget> HealthBarWidget;
};
