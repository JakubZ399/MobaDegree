// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMobaDegree, Log, All);

#define MOBALOG(Verbosity, bShowOnScreen, Format, ...) \
{ \
UE_LOG(LogTemp, Verbosity, Format, ##__VA_ARGS__); \
\
if (bShowOnScreen && GEngine) \
{ \
FColor LogColor = FColor::White; \
if (Verbosity == ELogVerbosity::Warning) LogColor = FColor::Yellow; \
else if (Verbosity == ELogVerbosity::Error) LogColor = FColor::Red; \
\
GEngine->AddOnScreenDebugMessage(-1, 5.0f, LogColor, FString::Printf(Format, ##__VA_ARGS__)); \
} \
}
