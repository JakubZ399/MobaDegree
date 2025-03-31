// 2025 Jakub Żurawik. All Rights Reserved.


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
