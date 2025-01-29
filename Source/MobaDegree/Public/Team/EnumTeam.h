// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EEnumTeam : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Blue    UMETA(DisplayName = "Blue"),
	Red     UMETA(DisplayName = "Red")
};
