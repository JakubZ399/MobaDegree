// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaDegreeGameMode.h"
#include "MobaDegreePlayerController.h"
#include "MobaDegreeCharacter.h"
#include "Component/TeamComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MobaPlayerState.h"
#include "UObject/ConstructorHelpers.h"

AMobaDegreeGameMode::AMobaDegreeGameMode()
{
}

void AMobaDegreeGameMode::ServerTravel(FString Map)
{
	if (GetWorld())
	{
		GetWorld()->ServerTravel(Map);
	}
}