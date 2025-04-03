// 2025 Jakub Żurawik. All Rights Reserved.


#include "Component/HealthComponent.h"

#include "AbilitySystemInterface.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Interfaces/UIInterface.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Daj czas innym komponentom na inicjalizację
    FTimerHandle InitTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &UHealthComponent::InitializeWithDelay, 0.2f, false);
}

void UHealthComponent::InitializeWithDelay()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    // Pobierz AbilitySystemComponent
    if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Owner))
    {
        OwnerASC = ASCInterface->GetAbilitySystemComponent();
        
        if (OwnerASC)
        {
            // Subskrybuj na zmiany zdrowia
            OwnerASC->GetGameplayAttributeValueChangeDelegate(UMobaAttributeSet::GetHealthAttribute())
                .AddUObject(this, &UHealthComponent::OnHealthAttributeChanged);
                
            OwnerASC->GetGameplayAttributeValueChangeDelegate(UMobaAttributeSet::GetMaxHealthAttribute())
                .AddUObject(this, &UHealthComponent::OnMaxHealthAttributeChanged);
            
            // Pobierz początkowe wartości
            bool Found = false;
            CurrentHealth = OwnerASC->GetGameplayAttributeValue(UMobaAttributeSet::GetHealthAttribute(), Found);
            MaxHealth = OwnerASC->GetGameplayAttributeValue(UMobaAttributeSet::GetMaxHealthAttribute(), Found);
        }
    }
    
    // Znajdź WidgetComponent
    OwnerHealthBar = Owner->FindComponentByClass<UWidgetComponent>();
    
    // Aktualizuj UI
    UpdateHealthBar();
}

void UHealthComponent::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
    CurrentHealth = Data.NewValue;
    UpdateHealthBar();
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
    MaxHealth = Data.NewValue;
    UpdateHealthBar();
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::UpdateHealthBar()
{
    if (OwnerHealthBar && OwnerHealthBar->GetWidget())
    {
        if (IUIInterface* UIInterface = Cast<IUIInterface>(OwnerHealthBar->GetWidget()))
        {
            float HealthPercent = (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
            IUIInterface::Execute_SetBarValue(OwnerHealthBar, HealthPercent);
        }
    }
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UHealthComponent, CurrentHealth);
    DOREPLIFETIME(UHealthComponent, MaxHealth);
}

