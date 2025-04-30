// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MobaDegreeGameMode.generated.h"

UCLASS(minimalapi)
class AMobaDegreeGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMobaDegreeGameMode();

	UFUNCTION(BlueprintCallable)
	void ServerTravel(FString Map = "/Game/Blueprints/Levels/Map?listen");
};



