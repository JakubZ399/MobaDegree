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

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1, EffectContextHandle);

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create valid SpecHandle for %s"), *GetName());
		return;
	}

	FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	bool Found;
	float MovementSpeedValue = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetMovementSpeedAttribute(), Found);
	if (Found)
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeedValue;
	}
	
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BaseAttack, 1));
}

void AMinionBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//OnMinionDeath.Broadcast(this);
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

void AMinionBase::ChangeMesh_Implementation()
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
	if (AttackTargetRef == nullptr)
	{
		if (GetBlackboardComponent())
		{
			GetBlackboardComponent()->SetValueAsObject(AttackTargetKey, nullptr);
			GetBlackboardComponent()->SetValueAsBool("FightWithOtherGroup", false);
			return;
		}
	}
	
	AttackTarget = AttackTargetRef;
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsObject(AttackTargetKey, AttackTarget);
		GetBlackboardComponent()->SetValueAsBool("FightWithOtherGroup", true);
	}
}

void AMinionBase::Attack()
{
	AbilitySystemComponent->TryActivateAbilityByClass(BaseAttack);

	/*if (GetWorld() && AttackTarget)
	{
		DrawDebugSphere(GetWorld(), AttackTarget->GetActorLocation(), 50.f, 12, FColor::Blue, false, 1.f);
	}*/
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
	bool Found = false;
	float Range = AbilitySystemComponent->GetGameplayAttributeValue(UMobaAttributeSet::GetAttackRangeAttribute(), Found);

	return Range;
}
