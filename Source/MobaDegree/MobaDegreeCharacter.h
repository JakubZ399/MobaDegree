// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/MobaInteraction.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MobaDegreeCharacter.generated.h"

class UMobaAbilitySystemComponent;
class AMobaPlayerState;
class UWidgetComponent;
class UHealthComponent;
class UPathFollowingComponent;
class UMobaAttributeSet;
class UMobaDefaultPlayerAttributeSet;
class UTeamComponent;

UCLASS(Blueprintable)
class AMobaDegreeCharacter : public ACharacter, public IAbilitySystemInterface, public IMobaTeamInterface, public IMobaInteraction
{
	GENERATED_BODY()

public:
	AMobaDegreeCharacter();
	virtual void Tick(float DeltaSeconds) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> AttackTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> OldAttackTarget;

	UFUNCTION()
	void MoveToLocation(FVector Location);

	virtual void ShowOutline_Implementation(bool EnableOutline, int32 OutlineColor) override;

protected:
	virtual void BeginPlay() override;

	void InitializeAttribute();

private:
	UPROPERTY()
	TObjectPtr<AMobaPlayerState> MobaPlayerState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	//Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;
	//
	
	UPROPERTY(EditAnywhere, Category = "Setup|GAS")
	TSubclassOf<UGameplayEffect> InitEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMobaAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UMobaAttributeSet> AttributeSet;

	UFUNCTION()
	void InitAbilityActorInfo();

public:
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintCallable)
	virtual EGameTeam GetTeamInterface_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};

