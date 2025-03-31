// Fill out your copyright notice in the Description page of Project Settings.


#include "Minions/MinionBase.h"

#include "AIController.h"
#include "AIHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/TeamComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AttributeSets/MobaAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Minions/MinionsGroupPawn.h"
#include "Misc/MapErrors.h"
#include "MobaDegree/MobaDegree.h"

AMinionBase::AMinionBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMobaAttributeSet>(TEXT("AttributeSet"));

	TeamComponent = CreateDefaultSubobject<UTeamComponent>("TeamComponent");
	TeamComponent->SetIsReplicated(true);

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("HealthBarWidgetComponent");
	HealthBarWidgetComponent->SetIsReplicated(true);

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
	if (!AttackTarget) return;
	
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
	AActor*& SelfRef)
{
	if (GetMesh())
	{
		MeshComponent = GetMesh();
		AttackTargetRef = AttackTarget;
		SelfRef = this;
	}
}

void AMinionBase::BeginPlay()
{
	Super::BeginPlay();

	//Initialize Attributes
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1 , EffectContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	//Setup MovementSpeed
	bool Found;
	float MovementSpeedValue = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetMovementSpeedAttribute(), Found);
	if (Found)
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeedValue;
	}
	
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BaseAttack, 1));
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
					//AIController->bStopAILogicOnUnposses = true;
					AIController->UnPossess();

					SetLifeSpan(0.2);

					UGameplayStatics::SpawnEmitterAtLocation(this, DeathParticle, GetActorLocation());
					
				}
			}
		}
	}

}

float AMinionBase::GetAttackRange()
{
	bool Found;
	return AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetAttackRangeAttribute(), Found);
}
