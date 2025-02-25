// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MobaTower.h"
#include "Component/AttackComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GAS/AttributeSets/MinionAttributeSet.h"

AMobaTower::AMobaTower()
{
	PrimaryActorTick.bCanEverTick = true;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>("Tower Mesh");
	SetRootComponent(TowerMesh);

	TowerRadius = CreateDefaultSubobject<USphereComponent>("Tower Radius");
	TowerRadius->SetupAttachment(TowerMesh);

	AttackComponent = CreateDefaultSubobject<UAttackComponent>("Attack Component");
	AttackComponent->SetIsReplicated(true);

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>("Projectile Spawner");
	ProjectileSpawner->SetupAttachment(GetRootComponent());

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMinionAttributeSet>("Attribute Set");
	
}

void AMobaTower::BeginPlay()
{
	Super::BeginPlay();

	TowerRadius->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAggroRangeBeginOverlap);
	TowerRadius->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAggroRangeEndOverlap);

	ProjectileSpawnerTransform = ProjectileSpawner->GetComponentTransform();

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1 , EffectContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	
}

void AMobaTower::OnAggroRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APawn>(OtherActor))
	{
		PawnsArray.AddUnique(OtherActor);
	}
	if (!AttackComponent->GetAttackTarget() && Cast<APawn>(OtherActor) && !bIsAttacking)
	{
			AttackComponent->SetAttackTarget(OtherActor);
			bIsAttacking = true;
			SpawnTowerShot();
	}
}

void AMobaTower::OnAggroRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AttackComponent->GetAttackTarget() && AttackComponent->GetAttackTarget() == OtherActor)
	{
		AttackComponent->SetAttackTarget(nullptr);
	}
}

void AMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UAbilitySystemComponent* AMobaTower::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

