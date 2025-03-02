// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/TeamComponent.h"

UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTeamComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UTeamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

EGameTeam UTeamComponent::GetTeamInterface_Implementation()
{
	return GetTeam();
}