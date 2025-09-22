// 2025 Jakub Żurawik. All Rights Reserved.


#include "GAS/AbilityTask/TargetDataScreenTrace.h"
#include "AbilitySystemComponent.h"

UTargetDataScreenTrace* UTargetDataScreenTrace::CreateTargetDataScreenTrace(UGameplayAbility* OwningAbility, float InTraceDistance)
{
    UTargetDataScreenTrace* Task = NewAbilityTask<UTargetDataScreenTrace>(OwningAbility);
    Task->TraceDistance = InTraceDistance;
    return Task;
}

void UTargetDataScreenTrace::Activate()
{
    const bool bLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
    if (bLocallyControlled)
    {
        SendScreenTraceData(); // klient przygotowuje i wysyła TargetData
    }
    else
    {
        const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
        const FPredictionKey PredKey = GetActivationPredictionKey();

        AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, PredKey)
            .AddUObject(this, &UTargetDataScreenTrace::OnTargetDataReplicatedCallback);

        const bool bAlreadySet = AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, PredKey);
        if (!bAlreadySet)
        {
            SetWaitingOnRemotePlayerData();
        }
    }
}

void UTargetDataScreenTrace::SendScreenTraceData()
{
    if (!AbilitySystemComponent.IsValid()) { EndTask(); return; }

    FScopedPredictionWindow ScopedPred(AbilitySystemComponent.Get());

    APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
    if (!PC) { EndTask(); return; }

    int32 SizeX=0, SizeY=0;
    PC->GetViewportSize(SizeX, SizeY);

    FVector WorldLoc, WorldDir;
    PC->DeprojectScreenPositionToWorld(SizeX * 0.5f, SizeY * 0.5f, WorldLoc, WorldDir);
    WorldDir.Normalize();

    const FVector EndLoc = WorldLoc + (WorldDir * TraceDistance);
    
    FGameplayAbilityTargetDataHandle Handle;
    Handle.Add(new FGameplayAbilityTargetData_ScreenTrace(WorldLoc, EndLoc));

    AbilitySystemComponent->ServerSetReplicatedTargetData(
        GetAbilitySpecHandle(),
        GetActivationPredictionKey(),
        Handle,
        FGameplayTag(),
        AbilitySystemComponent->ScopedPredictionKey);

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(Handle);
    }
}

void UTargetDataScreenTrace::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag /*ActivationTag*/)
{
    // Serwer (lub drugi koniec) konsumuje dane
    AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(DataHandle);
    }
    EndTask();
}
