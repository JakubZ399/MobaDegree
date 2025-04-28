// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "LobbyPlayerDetailsWidget.generated.h"

/**
 * 
 */
UCLASS()
class MOBADEGREE_API ULobbyPlayerDetailsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetupPlayerDetails(FString PlayerName);
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", BindWidget))
	TObjectPtr<UImage> PlayerHeroImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;
};
