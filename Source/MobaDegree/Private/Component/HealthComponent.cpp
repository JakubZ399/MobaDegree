// 2025 Jakub Żurawik. All Rights Reserved.

#include "Component/HealthComponent.h"

#include "AbilitySystemInterface.h"
#include "Components/TextBlock.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "MobaDegree/MobaDegreePlayerController.h"
#include "Player/MobaPlayerState.h"
#include "UI/Widget/HealthBarWidget.h"
#include "UI/Widget/MobaMainUserWidget.h"
#include "UI/Widget/PlayerStatBarWidget.h"

UHealthComponent::UHealthComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        if (IMobaTeamInterface* TeamInterface = Cast<IMobaTeamInterface>(Owner))
        {
            EGameTeam OwnerTeam = TeamInterface->Execute_GetTeamInterface(Owner);
        }
    }
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
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

            if (bIsPlayer)
            {
                if (ManaChangedDelegateHandle.IsValid())
                {
                    OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetManaAttribute()).Remove(ManaChangedDelegateHandle);
                }
            
                if (MaxManaChangedDelegateHandle.IsValid())
                {
                    OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxManaAttribute()).Remove(MaxManaChangedDelegateHandle);
                }
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
        HealthBarWidget->GetHealthProgressBar()->SetPercent(1.f);
    }
}

void UHealthComponent::HealthBarInitialization()
{
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerPawn);

    if (!AbilitySystemInterface)
    {
        AMobaPlayerState* PS = Cast<AMobaPlayerState>(OwnerPawn->GetPlayerState());
        if (PS)
        {
            AbilitySystemInterface = Cast<IAbilitySystemInterface>(PS);
        }
    }
    
    if (!AbilitySystemInterface) return;
    
    OwnerAbilitySystemComponent = Cast<UAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
    if (!OwnerAbilitySystemComponent) return;
    
    const UMobaAttributeSet* AttributeSet = Cast<UMobaAttributeSet>(OwnerAbilitySystemComponent->GetAttributeSet(UMobaAttributeSet::StaticClass()));
    if (!AttributeSet) return;
    
    bool bFound = false;
    Health = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetHealthAttribute(), bFound);
    MaxHealth = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMaxHealthAttribute(), bFound);
    
    HealthChangedDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
    .AddUObject(this, &UHealthComponent::OnHealthWidgetChange);

    MaxHealthChangedDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute())
    .AddUObject(this, &UHealthComponent::OnMaxHealthWidgetChange);

    if (bIsPlayer)
    {
        Mana = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetManaAttribute(), bFound);
        MaxMana = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMaxManaAttribute(), bFound);
        
        ManaChangedDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetManaAttribute())
        .AddUObject(this, &UHealthComponent::OnManaWidgetChange);

        MaxManaChangedDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxManaAttribute())
        .AddUObject(this, &UHealthComponent::OnMaxManaWidgetChange);

        UpdateManaWidget();
    }
    
    UpdateHealthBar();
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

    bool bFound = false;
    Health = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetHealthAttribute(), bFound);
    MaxHealth = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMaxHealthAttribute(), bFound);
    
    UpdateHealthBar();
}

void UHealthComponent::RefreshManaBar()
{
    if (!OwnerAbilitySystemComponent || !bIsPlayer)
    {
        return;
    }
    
    const UMobaAttributeSet* AttributeSet = Cast<UMobaAttributeSet>(OwnerAbilitySystemComponent->GetAttributeSet(UMobaAttributeSet::StaticClass()));
    if (!AttributeSet)
    {
        return;
    }

    bool bFound = false;
    Mana = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetManaAttribute(), bFound);
    MaxMana = OwnerAbilitySystemComponent->GetGameplayAttributeValue(AttributeSet->GetMaxManaAttribute(), bFound);
    
    UpdateManaWidget();
}

void UHealthComponent::SetHealthBarColor()
{
    if (OwnerPawn && OwnerPawn->GetClass()->ImplementsInterface(UMobaTeamInterface::StaticClass()))
    {
        EGameTeam Team = IMobaTeamInterface::Execute_GetTeamInterface(OwnerPawn);

        FTimerHandle TimerHandle;

        if (HealthBarWidget)
        {
            HealthBarWidget->SetBarColor(Team);
        }
    }
}

void UHealthComponent::OnHealthWidgetChange(const FOnAttributeChangeData& Data)
{
    Health = Data.NewValue;
    UpdateHealthBar();
}

void UHealthComponent::OnMaxHealthWidgetChange(const FOnAttributeChangeData& Data)
{
    MaxHealth = Data.NewValue;
    UpdateHealthBar();
}

void UHealthComponent::OnManaWidgetChange(const FOnAttributeChangeData& Data)
{
    Mana = Data.NewValue;
    UpdateManaWidget();
}

void UHealthComponent::OnMaxManaWidgetChange(const FOnAttributeChangeData& Data)
{
    MaxMana = Data.NewValue;
    UpdateManaWidget();
}

void UHealthComponent::UpdateManaWidget()
{
    if (!HealthBarWidget) return;
    if (!bIsPlayer) return;
    
    if (MaxMana > 0)
    {
        float ManaPercent = FMath::Clamp(Mana / MaxMana, 0.f, 1.f);

        if (AMobaDegreePlayerController* PlayerController = Cast<AMobaDegreePlayerController>(OwnerPawn->GetController()))
        {
            if (PlayerController->MainUserWidget)
            {
                if (PlayerController->MainUserWidget->ManaBar)
                {
                    PlayerController->MainUserWidget->ManaBar->SetTextStatValue(Mana);
                    PlayerController->MainUserWidget->ManaBar->SetTextStatMaxValue(MaxMana);
                }
            }
        }
    }
}

void UHealthComponent::UpdateHealthBar()
{
    if (!HealthBarWidget) return;
    
    if (MaxHealth > 0)
    {
        float HealthPercent = FMath::Clamp(Health / MaxHealth, 0.f, 1.f);
        
        HealthBarWidget->GetHealthProgressBar()->SetPercent(HealthPercent);

        if (AMobaDegreePlayerController* PlayerController = Cast<AMobaDegreePlayerController>(OwnerPawn->GetController()))
        {
            if (PlayerController->MainUserWidget)
            {
                if (PlayerController->MainUserWidget->HealthBar)
                {
                    PlayerController->MainUserWidget->HealthBar->SetTextStatValue(Health);
                    PlayerController->MainUserWidget->HealthBar->SetTextStatMaxValue(MaxHealth);
                }
            }
        }
    }
}
