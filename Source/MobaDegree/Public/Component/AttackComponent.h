// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBADEGREE_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttackComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<AActor> AttackTarget;
protected:
	virtual void BeginPlay() override;



public:
	FORCEINLINE void SetAttackTarget(AActor* Target) { AttackTarget = Target; };
	FORCEINLINE AActor* GetAttackTarget() { return AttackTarget; };
		
};
