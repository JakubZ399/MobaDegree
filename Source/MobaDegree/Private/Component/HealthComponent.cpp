// 2025 Jakub Żurawik. All Rights Reserved.

#include "Component/HealthComponent.h"

#include "AbilitySystemInterface.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Player/MobaPlayerState.h"
#include "UI/Widget/HealthBarWidget.h"

UHealthComponent::UHealthComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Czyścimy delegaty przy zniszczeniu komponentu
    if (OwnerAbilitySystemComponent)
    {
        const UMobaAttributeSet* AttributeSet = Cast<UMobaAttributeSet>(OwnerAbilitySystemComponent->GetAttributeSet(UMobaAttributeSet::StaticClass()));
        if (AttributeSet)
        {
            if (HealthChangedDelegateHandle.IsValid())
            {
                OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
            }
            
            if (MaxHealthChangedDelegateHandle.IsValid())
            {
                OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).Remove(MaxHealthChangedDelegateHandle);
            }
        }
    }
}

void UHealthComponent::SetHealthBarWidgetFromOwner(UWidgetComponent* Widget)
{
    OwnerHealthBar = Widget;

    HealthBarWidget = Cast<UHealthBarWidget>(OwnerHealthBar->GetWidget());

    if (HealthBarWidget)
    {
        // Ustawiamy początkową wartość na 1.0 (100%)
        HealthBarWidget->GetHealthProgressBar()->SetPercent(1.f);
    }
}

void UHealthComponent::HealthBarInitialization()
{
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("HealthBarInitialization: Owner is not a Pawn!"));
        return;
    }
    
    // Próbujemy pobrać ASC z różnych źródeł, zależnie od typu aktora
    IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerPawn);
    
    // Jeśli nie znaleźliśmy ASC bezpośrednio, próbujemy z PlayerState (dla graczy)
    if (!AbilitySystemInterface)
    {
        // Tylko dla postaci gracza (ma PlayerState)
        AMobaPlayerState* PS = Cast<AMobaPlayerState>(OwnerPawn->GetPlayerState());
        if (PS)
        {
            AbilitySystemInterface = Cast<IAbilitySystemInterface>(PS);
        }
    }
    
    if (!AbilitySystemInterface)
    {
        UE_LOG(LogTemp, Error, TEXT("HealthBarInitialization: Could not find AbilitySystemInterface on %s"), *OwnerPawn->GetName());
        return;
    }
    
    OwnerAbilitySystemComponent = Cast<UAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
    if (!OwnerAbilitySystemComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("HealthBarInitialization: Could not find AbilitySystemComponent on %s"), *OwnerPawn->GetName());
        return;
    }
    
    const UMobaAttributeSet* AttributeSet = Cast<UMobaAttributeSet>(OwnerAbilitySystemComponent->GetAttributeSet(UMobaAttributeSet::StaticClass()));
    if (!AttributeSet)
    {
        UE_LOG(LogTemp, Error, TEXT("HealthBarInitialization: Could not find MobaAttributeSet on %s"), *OwnerPawn->GetName());
        return;
    }
    
    // Pobieramy początkowe wartości
    bool bFound = false;
    Health = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetHealthAttribute(), bFound);
    MaxHealth = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMaxHealthAttribute(), bFound);
    
    // Rejestrujemy delegaty na zmiany atrybutów
    HealthChangedDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
        .AddUObject(this, &UHealthComponent::OnHealthWidgetChange);
    
    MaxHealthChangedDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute())
        .AddUObject(this, &UHealthComponent::OnMaxHealthWidgetChange);
    
    // Aktualizujemy UI od razu z aktualnymi wartościami
    UpdateHealthBar();
    
    UE_LOG(LogTemp, Warning, TEXT("HealthBarInitialization: Successfully initialized for %s. Health: %.2f/%.2f"), 
        *OwnerPawn->GetName(), Health, MaxHealth);
}

void UHealthComponent::RefreshHealthBar()
{
    if (!OwnerAbilitySystemComponent)
    {
        return;
    }
    
    const UMobaAttributeSet* AttributeSet = Cast<UMobaAttributeSet>(OwnerAbilitySystemComponent->GetAttributeSet(UMobaAttributeSet::StaticClass()));
    if (!AttributeSet)
    {
        return;
    }
    
    // Pobieramy aktualne wartości
    bool bFound = false;
    Health = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetHealthAttribute(), bFound);
    MaxHealth = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMaxHealthAttribute(), bFound);
    
    // Aktualizujemy UI
    UpdateHealthBar();
}

void UHealthComponent::OnHealthWidgetChange(const FOnAttributeChangeData& Data)
{
    Health = Data.NewValue;
    
    UE_LOG(LogTemp, Warning, TEXT("Health changed from %.2f to %.2f (MaxHealth = %.2f) on %s"), 
        Data.OldValue, Health, MaxHealth, OwnerPawn && OwnerPawn->HasAuthority() ? TEXT("Server") : TEXT("Client"));
    
    UpdateHealthBar();
}

void UHealthComponent::OnMaxHealthWidgetChange(const FOnAttributeChangeData& Data)
{
    MaxHealth = Data.NewValue;
    
    UE_LOG(LogTemp, Warning, TEXT("MaxHealth changed from %.2f to %.2f on %s"), 
        Data.OldValue, MaxHealth, OwnerPawn && OwnerPawn->HasAuthority() ? TEXT("Server") : TEXT("Client"));
    
    UpdateHealthBar();
}

void UHealthComponent::UpdateHealthBar()
{
    if (!HealthBarWidget)
    {
        return;
    }
    
    if (MaxHealth > 0)
    {
        float HealthPercent = FMath::Clamp(Health / MaxHealth, 0.f, 1.f);
        
        UE_LOG(LogTemp, Warning, TEXT("UpdateHealthBar: Setting health percent to %.2f (%.2f/%.2f) on %s"), 
            HealthPercent, Health, MaxHealth, OwnerPawn && OwnerPawn->HasAuthority() ? TEXT("Server") : TEXT("Client"));
        
        HealthBarWidget->GetHealthProgressBar()->SetPercent(HealthPercent);
    }
}