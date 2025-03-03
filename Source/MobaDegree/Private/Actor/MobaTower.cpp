// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MobaTower.h"
#include "Component/AttackComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Component/TeamComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"

AMobaTower::AMobaTower()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(CapsuleComponent);

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>("Tower Mesh");
	TowerMesh->SetupAttachment(GetRootComponent());

	TowerRadius = CreateDefaultSubobject<USphereComponent>("Tower Radius");
	TowerRadius->SetupAttachment(GetRootComponent());

	AttackComponent = CreateDefaultSubobject<UAttackComponent>("Attack Component");
	AttackComponent->SetIsReplicated(true);

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>("Projectile Spawner");
	ProjectileSpawner->SetupAttachment(GetRootComponent());

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMobaAttributeSet>("Attribute Set");

	TeamComponent = CreateDefaultSubobject<UTeamComponent>("TeamComponent");
	TeamComponent->SetIsReplicated(true);
	
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
	/*if (Cast<APawn>(OtherActor))
	{
		PawnsArray.AddUnique(OtherActor);
	}*/
	if (!AttackComponent->GetAttackTarget() && Cast<APawn>(OtherActor) && !bIsAttacking)
	{
		/*IMobaTeamInterface* TeamInterface = Cast<IMobaTeamInterface>(OtherActor);
		if (TeamInterface && TeamInterface->GetTeamInterface_Implementation() != TeamComponent->GetTeam())
		{*/
			AttackComponent->SetAttackTarget(OtherActor);
			bIsAttacking = true;
			SpawnTowerShot();
		/*}*/
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

EGameTeam AMobaTower::GetTeamInterface_Implementation() const
{
	return TeamComponent->GetTeam();
}

void AMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UAbilitySystemComponent* AMobaTower::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

