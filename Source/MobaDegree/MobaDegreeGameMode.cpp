// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaDegreeGameMode.h"
#include "MobaDegreePlayerController.h"
#include "MobaDegreeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMobaDegreeGameMode::AMobaDegreeGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AMobaDegreePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/Player/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}