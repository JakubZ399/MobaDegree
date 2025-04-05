// 2025 Jakub Żurawik. All Rights Reserved.


#include "Component/HealthComponent.h"
#include "UI/Widget/HealthBarWidget.h"

UHealthComponent::UHealthComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHealthComponent::SetHealthBarWidgetFromOwner(UWidgetComponent* Widget)
{
    OwnerHealthBar = Widget;

    HealthBarWidget = Cast<UHealthBarWidget>(OwnerHealthBar->GetWidget());

    HealthBarWidget->GetHealthProgressBar()->SetPercent(0.5);
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

}