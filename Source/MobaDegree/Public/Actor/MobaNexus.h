// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MobaBasePawn.h"
#include "GameFramework/Pawn.h"
#include "MobaNexus.generated.h"

UCLASS()
class MOBADEGREE_API AMobaNexus : public AMobaBasePawn
{
	GENERATED_BODY()

public:
	AMobaNexus();

protected:
	virtual void BeginPlay() override;
};
