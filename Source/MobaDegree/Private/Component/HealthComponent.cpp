// 2025 Jakub Żurawik. All Rights Reserved.

#include "Component/HealthComponent.h"

#include "AbilitySystemInterface.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "UI/Widget/HealthBarWidget.h"

UHealthComponent::UHealthComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHealthComponent::SetHealthBarWidgetFromOwner(UWidgetComponent* Widget)
{
    OwnerHealthBar = Widget;

    HealthBarWidget = Cast<UHealthBarWidget>(OwnerHealthBar->GetWidget());

    if (HealthBarWidget)
    {
        HealthBarWidget->GetHealthProgressBar()->SetPercent(1.f);
    }
}

void UHealthComponent::HealthBarInitialization()
{
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerPawn);
        if (AbilitySystemInterface)
        {
            OwnerAbilitySystemComponent = Cast<UAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
            if (OwnerAbilitySystemComponent)
            {
                const UMobaAttributeSet* AttributeSet = Cast<UMobaAttributeSet>(OwnerAbilitySystemComponent->GetAttributeSet(UMobaAttributeSet::StaticClass()));
                if (AttributeSet)
                {
                    auto HealthChangeDelegate = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
                    .AddUObject(this, &UHealthComponent::OnHealthWidgetChange);

                    MaxHealth = AttributeSet->GetMaxHealth();
                }
            }
        }
    }
}

void UHealthComponent::OnHealthWidgetChange(const FOnAttributeChangeData& Data)
{
    float CurrentHealth = Data.NewValue;
    float OldHealth = Data.OldValue;

    UE_LOG(LogTemp, Warning, TEXT("Health changed from %.2f to %.2f (MaxHealth = %.2f) on %s"), 
          OldHealth, CurrentHealth, MaxHealth, OwnerPawn->HasAuthority() ? TEXT("Server") : TEXT("Client"));

    
    if (MaxHealth > 0)
    {
        float HealthPercent = FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f);

        if (HealthBarWidget)
        {
            HealthBarWidget->GetHealthProgressBar()->SetPercent(HealthPercent);
        }
    }
}
