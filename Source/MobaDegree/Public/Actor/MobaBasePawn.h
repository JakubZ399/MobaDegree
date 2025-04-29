// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MobaBasePawn.generated.h"

class UCapsuleComponent;
class UGameplayEffect;
class UWidgetComponent;
class UHealthComponent;
class UTeamComponent;
class UMobaAttributeSet;

UCLASS()
class MOBADEGREE_API AMobaBasePawn : public APawn, public IAbilitySystemInterface, public IMobaTeamInterface, public IMobaInteraction
{
	GENERATED_BODY()

public:
	AMobaBasePawn();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ShowOutline_Implementation(bool EnableOutline) override;

protected:
	virtual void BeginPlay() override;
	void InitializeAttributes();

	UPROPERTY(ReplicatedUsing = OnRep_bAttributeInitialized)
	bool bAttributeInitialized = false;

	UFUNCTION() void OnRep_bAttributeInitialized();

	
#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasePawn|Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasePawn|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasePawn|Components")
	TObjectPtr<UAbilitySystemComponent> MobaAbilitySystemComponent;

	UPROPERTY(Instanced)
	TObjectPtr<UMobaAttributeSet> MobaAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasePawn|Components")
	TObjectPtr<UTeamComponent> TeamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasePawn|Components")
	TObjectPtr<UHealthComponent> HealthComponent;
#pragma endregion Components

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasePawn|Setup")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasePawn|Setup")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasePawn|Setup")
	TSubclassOf<UGameplayEffect> InitEffect;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual EGameTeam GetTeamInterface_Implementation() const override;
	
};
