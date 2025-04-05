// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Interfaces/UIInterface.h"
#include "HealthBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class MOBADEGREE_API UHealthBarWidget : public UUserWidget, public IUIInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void SetBarValue_Implementation(float BarValue) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget="HealthProgressBar"))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget="HealthBarBox"))
	TObjectPtr<USizeBox> HealthBarBox;

	UFUNCTION(BlueprintCallable)
	void SetupProgressBar(ESlateBrushRoundingType::Type RoundingType, float Width);

	UFUNCTION(BlueprintCallable)
	void SetupSizeBox(float Width, float Height);
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UProgressBar* GetHealthProgressBar() { return HealthProgressBar.Get(); }
};
