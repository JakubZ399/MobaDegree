// 2025 Jakub Żurawik. All Rights Reserved.


#include "Menu/Lobby/Widgets/LobbyPlayerDetailsWidget.h"

void ULobbyPlayerDetailsWidget::SetupPlayerDetails(FString PlayerName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PlayerName));
	}
}
