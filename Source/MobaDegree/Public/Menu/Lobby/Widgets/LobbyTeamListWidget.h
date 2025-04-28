// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyTeamListWidget.generated.h"

class UVerticalBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class MOBADEGREE_API ULobbyTeamListWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", BindWidget))
	TObjectPtr<UVerticalBox> PlayerListBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", BindWidget))
	TObjectPtr<UTextBlock> ChooseTeamText;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UVerticalBox* GetPlayerListBox() { return PlayerListBox; }
};
