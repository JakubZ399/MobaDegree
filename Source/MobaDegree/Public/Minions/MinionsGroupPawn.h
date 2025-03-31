// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MinionsGroupPawn.generated.h"

class AMinionBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackTargetSet, AActor*, AttackTarget);

UCLASS()
class MOBADEGREE_API AMinionsGroupPawn : public APawn
{
	GENERATED_BODY()

public:
	AMinionsGroupPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackTargetSet OnAttackTargetSet;

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Setup")
	TSubclassOf<AMinionBase> MeleeMinionClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Setup")
	TSubclassOf<AMinionBase> RangedMinionClass;

};
