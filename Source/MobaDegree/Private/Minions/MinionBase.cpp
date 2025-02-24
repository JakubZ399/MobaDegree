// Fill out your copyright notice in the Description page of Project Settings.


#include "Minions/MinionBase.h"

AMinionBase::AMinionBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AMinionBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMinionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


