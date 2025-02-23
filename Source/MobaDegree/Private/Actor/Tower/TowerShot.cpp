// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Tower/TowerShot.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ATowerShot::ATowerShot()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

}

void ATowerShot::AttackTarget(AActor* Target)
{
	if (Target)
	{
		AttackTargetActor = Target;
		GetWorld()->GetTimerManager().SetTimer(LerpTimer, this, &ATowerShot::UpdateAlpha, 0.005f, true);
		
	}
}

void ATowerShot::UpdateAlpha()
{
	AlphaToInterpolation += 0.003f;
	AlphaToInterpolation = FMath::Clamp(AlphaToInterpolation, 0.0f, 1.0f);

	// Przyspieszenie pod koniec (Exponent = 2.5f -> im wyższa wartość, tym mocniejsze przyspieszenie)
	float EasedAlpha = FMath::InterpExpoIn(0.0f, 1.0f, AlphaToInterpolation);


	FVector ShotLocation = FMath::Lerp(GetActorLocation(), AttackTargetActor->GetActorLocation(), EasedAlpha);
	SetActorLocation(ShotLocation);

	if (AlphaToInterpolation >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LerpTimer);
	}
}


