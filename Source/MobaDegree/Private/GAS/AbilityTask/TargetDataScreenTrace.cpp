// 2025 Jakub Żurawik. All Rights Reserved.


#include "GAS/AbilityTask/TargetDataScreenTrace.h"

UTargetDataScreenTrace* UTargetDataScreenTrace::GetTargetDataFromScreenTrace(UGameplayAbility* OwningAbility, float TraceDistance)
{
	UTargetDataScreenTrace* MyObj = NewAbilityTask<UTargetDataScreenTrace>(OwningAbility);
	MyObj->TraceDistance = TraceDistance;
	return MyObj;
}

void UTargetDataScreenTrace::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendDataFromScreenTrace();
	}
	else
	{
		
	}
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);

	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(SizeX / 2, SizeY / 2, WorldLocation, WorldDirection);

	const FVector EndLocation = WorldLocation + (WorldDirection * TraceDistance);

	ValidData.Broadcast(WorldLocation, EndLocation);
}

void UTargetDataScreenTrace::SendDataFromScreenTrace()
{
}
