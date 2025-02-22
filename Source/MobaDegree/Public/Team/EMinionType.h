#pragma once

#include "EMinionType.generated.h"

UENUM(BlueprintType)
enum class EMinionType : uint8
{
	Melee   UMETA(DisplayName="Melee"),
	Ranged  UMETA(DisplayName="Ranged"),
	Canon   UMETA(DisplayName="Canon"),
	// Możesz dodać kolejne typy
};
