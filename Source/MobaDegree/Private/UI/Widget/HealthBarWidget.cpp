// 2025 Jakub Żurawik. All Rights Reserved.


#include "UI/Widget/HealthBarWidget.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::SetBarValue_Implementation(float BarValue)
{
	HealthProgressBar->SetPercent(BarValue);
}


