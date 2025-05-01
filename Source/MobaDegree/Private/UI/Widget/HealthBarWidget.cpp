// 2025 Jakub Żurawik. All Rights Reserved.


#include "UI/Widget/HealthBarWidget.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::SetBarValue_Implementation(float BarValue)
{
	HealthProgressBar->SetPercent(BarValue);
}

void UHealthBarWidget::SetBarColor(EGameTeam GameTeam)
{
	FTimerHandle TimerHandle;
	
	switch (GameTeam)
	{
	case EGameTeam::Blue:
		HealthProgressBar->SetFillColorAndOpacity(BlueTeamBarColor);
		break;

	case EGameTeam::Red:
		HealthProgressBar->SetFillColorAndOpacity(RedTeamBarColor);
		break;

	case EGameTeam::None:
		HealthProgressBar->SetFillColorAndOpacity(NoneTeamBarColor);
		
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,
			[this, GameTeam]()
			{
				SetBarColor(GameTeam);
			},
			.5f,
			false
			);
		break;
		
	default:
		break;
	}
}

void UHealthBarWidget::SetupProgressBar(ESlateBrushRoundingType::Type RoundingType, float Width)
{
	if (HealthProgressBar)
	{
		auto WidgetStyle = HealthProgressBar->GetWidgetStyle();
		WidgetStyle.FillImage.OutlineSettings.RoundingType = RoundingType;
		WidgetStyle.FillImage.OutlineSettings.Width = Width;
		WidgetStyle.BackgroundImage.OutlineSettings.RoundingType = RoundingType;
		WidgetStyle.BackgroundImage.OutlineSettings.Width = Width;
		HealthProgressBar->SetWidgetStyle(WidgetStyle);
	}
}

void UHealthBarWidget::SetupSizeBox(const float Width,const float Height)
{
	if (HealthBarBox)
	{
		HealthBarBox->SetWidthOverride(Width);
		HealthBarBox->SetHeightOverride(Height);
	}
}


