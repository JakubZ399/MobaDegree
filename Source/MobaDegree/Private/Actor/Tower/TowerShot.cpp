// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Tower/TowerShot.h"

#include "AbilitySystemInterface.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

ATowerShot::ATowerShot()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh Component");
	SetRootComponent(MeshComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>("Sphere Component");
	SphereComponent->SetupAttachment(GetRootComponent());
}

void ATowerShot::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!AttackTargetActor)
	{
		Destroy();
		return;
	}
	
	float EasedAlpha = FMath::InterpExpoIn(0.0f, 1.0f, AlphaToInterpolation);
	FVector ShotLocation = FMath::Lerp(GetActorLocation(), AttackTargetActor->GetActorLocation(), EasedAlpha);
	SetActorLocation(ShotLocation);
}

void ATowerShot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATowerShot, AlphaToInterpolation);
	DOREPLIFETIME(ATowerShot, AttackTargetActor);
}

void ATowerShot::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATowerShot::SphereOverlap);
}

void ATowerShot::AttackTarget(AActor* Target)
{
	if (Target)
	{
		AttackTargetActor = Target;
		GetWorld()->GetTimerManager().SetTimer(LerpTimer, this, &ATowerShot::UpdateAlpha, 0.005f, true);

		Target->OnDestroyed.AddDynamic(this, &ATowerShot::OnTargetDestroyCallback);
	}
}

void ATowerShot::OnTargetDestroyCallback(AActor* DestroyedActor)
{
	SetLifeSpan(0.25f);
}

void ATowerShot::UpdateAlpha()
{
	if (HasAuthority())
	{
		AlphaToInterpolation += TowerShotSpeed;
		AlphaToInterpolation = FMath::Clamp(AlphaToInterpolation, 0.0f, 1.0f);

		if (AlphaToInterpolation >= 1.0f)
		{
			GetWorld()->GetTimerManager().ClearTimer(LerpTimer);
		}
	}
}

void ATowerShot::SphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == AttackTargetActor && Cast<IAbilitySystemInterface>(OtherActor))
	{
		if (UAbilitySystemComponent* TargetASC = Cast<IAbilitySystemInterface>(OtherActor)->GetAbilitySystemComponent())
		{
			if (TowerShotEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(TowerShotEffect, 1, TargetASC->MakeEffectContext());
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				Destroy();
			}

		}
	}
}


