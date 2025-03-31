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

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget="HealthProgressBar"), meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget="HealthBarBox"), meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USizeBox> HealthBarBox;
};
