// 2025 Jakub Żurawik. All Rights Reserved.


#include "Component/TeamComponent.h"

#include "Net/UnrealNetwork.h"

UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTeamComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTeamComponent, Team);
}

void UTeamComponent::SetTeam_Implementation(EGameTeam TeamToChose)
{
	Team = TeamToChose;
}
