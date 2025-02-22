#pragma once

#include "EGameTeam.generated.h"

UENUM(BlueprintType)
enum class EGameTeam : uint8
{
	Blue  UMETA(DisplayName="Blue"),
	Red   UMETA(DisplayName="Red"),
	None  UMETA(DisplayName="None")
};
