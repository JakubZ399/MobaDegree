// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Components/WidgetComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBADEGREE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	UHealthComponent();
    
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

protected:
	virtual void BeginPlay() override;
    
	UFUNCTION()
	void InitializeWithDelay();
    
	// Callback na zmiany zdrowia z GAS
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data);
    
	// Aktualizuje HealthBar Widget
	void UpdateHealthBar();

private:
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;
    
	UPROPERTY()
	UWidgetComponent* OwnerHealthBar;
    
	UPROPERTY(Replicated)
	float CurrentHealth;
    
	UPROPERTY(Replicated)
	float MaxHealth;
};
