// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MobaTower.h"
#include "Component/AttackComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

AMobaTower::AMobaTower()
{
	PrimaryActorTick.bCanEverTick = true;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>("Tower Mesh");
	SetRootComponent(TowerMesh);

	TowerRadius = CreateDefaultSubobject<USphereComponent>("Tower Radius");
	TowerRadius->SetupAttachment(TowerMesh);

	AttackComponent = CreateDefaultSubobject<UAttackComponent>("Attack Component");
	
}

void AMobaTower::BeginPlay()
{
	Super::BeginPlay();

	TowerRadius->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAggroRangeBeginOverlap);
	TowerRadius->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAggroRangeEndOverlap);
	
}

void AMobaTower::OnAggroRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ACharacter>(OtherActor))
	{
		AttackComponent->SetAttackTarget(OtherActor);
		SpawnTowerShot();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Attack"));
		}
	}
	

}

void AMobaTower::OnAggroRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ACharacter>(OtherActor))
	{
		AttackComponent->SetAttackTarget(nullptr);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Cyan, TEXT("Stop"));
		}
	}
}

void AMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

