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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthBarWidgetFromOwner(UWidgetComponent* Widget);

	void HealthBarInitialization();
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void RefreshHealthBar();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void RefreshManaBar();

	UFUNCTION(BlueprintCallable)
	void SetHealthBarColor();

	//Setup this on Player character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	bool bIsPlayer{false};

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UWidgetComponent> OwnerHealthBar;

	UPROPERTY()
	TObjectPtr<UHealthBarWidget> HealthBarWidget;

	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerAbilitySystemComponent;

	void OnHealthWidgetChange(const FOnAttributeChangeData& Data);
	void OnMaxHealthWidgetChange(const FOnAttributeChangeData& Data);

	void OnManaWidgetChange(const FOnAttributeChangeData& Data);
	void OnMaxManaWidgetChange(const FOnAttributeChangeData& Data);
	
	void UpdateHealthBar();
	void UpdateManaWidget();

	float Health{0.f};
	float MaxHealth{0.f};

	float Mana{0.f};
	float MaxMana{0.f};
    
	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;

	FDelegateHandle ManaChangedDelegateHandle;
	FDelegateHandle MaxManaChangedDelegateHandle;
};