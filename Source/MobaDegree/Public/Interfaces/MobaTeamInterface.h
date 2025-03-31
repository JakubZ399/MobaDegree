// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MobaTeamInterface.generated.h"

enum class EGameTeam : uint8;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UMobaTeamInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MOBADEGREE_API IMobaTeamInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EGameTeam GetTeamInterface() const;
};
