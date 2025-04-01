// 2025 Jakub Żurawik. All Rights Reserved.

#include "Minions/MinionBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/TeamComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Minions/MinionsGroupPawn.h"

AMinionBase::AMinionBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	MobaAttributeSet = CreateDefaultSubobject<UMobaAttributeSet>(TEXT("MobaAttributeSet"));

	TeamComponent = CreateDefaultSubobject<UTeamComponent>("TeamComponent");
	TeamComponent->SetIsReplicated(true);

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("HealthBarWidgetComponent");
	HealthBarWidgetComponent->SetIsReplicated(true);

}

void AMinionBase::BeginPlay()
{
	Super::BeginPlay();

	checkf(AbilitySystemComponent, TEXT("AbilitySystemComponent is not set on Minion!"));
	checkf(InitEffect, TEXT("InitEffect is not set on Minion!"));
	checkf(MobaAttributeSet, TEXT("AttributeSet is not set on Minion!"));
	
	if (!InitEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("InitEffect is null for %s"), *GetName());
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1, EffectContextHandle);

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create valid SpecHandle for %s"), *GetName());
		return;
	}

	FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (!ActiveGEHandle.IsValid())
	{
		// Sprawdźmy, czy mimo błędu wartość jest poprawna
		bool Found;
		float Value = AbilitySystemComponent->GetGameplayAttributeValue(
			UMobaAttributeSet::GetAttackRangeAttribute(), Found);
    
		if (Found && Value == 400.0f)
		{
			// Wartość jest prawidłowa, więc ignorujemy błąd
			UE_LOG(LogTemp, Warning, TEXT("GE error on %s but attributes are correct"), *GetName());
		}
		else
		{
			// Jest prawdziwy problem z wartościami
			UE_LOG(LogTemp, Error, TEXT("Failed to apply GameplayEffect on %s"), *GetName());
		}
	}

	bool FoundTest;
	float AttackRangeValue = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetAttackRangeAttribute(), FoundTest);
	UE_LOG(LogTemp, Warning, TEXT("AttackRange after initialization: %f"), AttackRangeValue);

	//Setup MovementSpeed
	bool Found;
	float MovementSpeedValue = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetMovementSpeedAttribute(), Found);
	if (Found)
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeedValue;
	}
	
	float RangeFromGetter = GetAttackRange();
	UE_LOG(LogTemp, Warning, TEXT("%s: GetAttackRange() = %f vs direct = %f"), 
		   *GetName(), RangeFromGetter, AttackRangeValue);
	
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BaseAttack, 1));
}

void AMinionBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnMinionDeath.Broadcast();
}


void AMinionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HomeBase)
	{
		SetGroupPosition(HomeBase->GetComponentLocation());

		bool Found;
		float HealthValue = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetHealthAttribute(), Found);
		if (HealthValue <= 0)
		{
			if (!DoOnce)
			{
				DoOnce = true;

				AAIController* AIController = Cast<AAIController>(GetController());
				if (AIController)
				{
					AIController->UnPossess();

					SetLifeSpan(0.2);

					UGameplayStatics::SpawnEmitterAtLocation(this, DeathParticle, GetActorLocation());
				}
			}
		}
	}
}

UAbilitySystemComponent* AMinionBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

EGameTeam AMinionBase::GetTeamInterface_Implementation() const
{
	return TeamComponent->GetTeam();
}

void AMinionBase::ChangeMesh()
{
	EGameTeam MinionTeam = TeamComponent ? TeamComponent->GetTeam() : EGameTeam::None;
	
	if (!GetMesh()) return;
	
	switch (MinionTeam)
	{
	case EGameTeam::Blue:
		if (BlueMinionMesh)
		{
			GetMesh()->SetSkeletalMeshAsset(BlueMinionMesh);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SETUP: BlueMinionMesh is not set!"));
		}
		break;
	case EGameTeam::Red:
		if (RedMinionMesh)
		{
			GetMesh()->SetSkeletalMeshAsset(RedMinionMesh);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SETUP: RedMinionMesh is not set!"));
		}
	}
}

void AMinionBase::BindOnAttackTarget(class AMinionsGroupPawn* MinionsGroup)
{
	MinionsGroup->OnAttackTargetSet.AddDynamic(this, &AMinionBase::SetupAttackTarget);
}

void AMinionBase::SetupAttackTarget(AActor* AttackTargetRef)
{
	//if (!AttackTarget) return;
	
	AttackTarget = AttackTargetRef;

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsObject(AttackTargetKey, AttackTarget);
	}
}

void AMinionBase::Attack()
{
	AbilitySystemComponent->TryActivateAbilityByClass(BaseAttack);

	if (GetWorld())
	{
		DrawDebugSphere(GetWorld(), AttackTarget->GetActorLocation(), 50.f, 12, FColor::Blue, true, 2.f);
	}
}

void AMinionBase::SetGroupPosition(FVector GroupPositionRef)
{
	GroupPosition = GroupPositionRef;

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(GroupPositionKey, GroupPosition);
	}
}

UBlackboardComponent* AMinionBase::GetBlackboardComponent()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController) return nullptr;

	UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
	if (!BlackboardComponent) return nullptr;

	return BlackboardComponent;
}

void AMinionBase::ShowOutline_Implementation(bool EnableOutline)
{
	IMobaInteraction::ShowOutline_Implementation(EnableOutline);

	GetMesh()->SetRenderCustomDepth(EnableOutline);
}

void AMinionBase::CallOnAttackEndInterface_Implementation()
{
	IMinionAI::CallOnAttackEndInterface_Implementation();

	OnAttackEnd.Broadcast();
}

AMinionBase* AMinionBase::GetMinionRef_Implementation()
{
	return this;
}

float AMinionBase::GetAttackRadiusAttribute_Implementation()
{
	return GetAttackRange();
}

void AMinionBase::EnemyInfoAI_Implementation(USkeletalMeshComponent* &MeshComponent, AActor*& AttackTargetRef,
	AMinionBase*& SelfRef)
{
	if (GetMesh())
	{
		MeshComponent = GetMesh();
		AttackTargetRef = AttackTarget;
		SelfRef = this;
	}
}



float AMinionBase::GetAttackRange()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s has no AbilitySystemComponent!"), *GetName());
		return 500.f; // Wartość fallback
	}
    
	bool Found = false;
	float Range = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetAttackRangeAttribute(), Found);
    
	UE_LOG(LogTemp, Display, TEXT("%s GetAttackRange() returning: %f (Found: %d)"), 
		   *GetName(), Found ? Range : 500.f, Found ? 1 : 0);
    
	return Found ? Range : 500.f;
}
