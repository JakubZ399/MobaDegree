// 2025 Jakub Żurawik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/MobaTeamInterface.h"
#include "MobaDegreeCharacter.generated.h"

class UWidgetComponent;
class UHealthComponent;
class UPathFollowingComponent;
class UMobaAttributeSet;
class UMobaDefaultPlayerAttributeSet;
class UTeamComponent;

UCLASS(Blueprintable)
class AMobaDegreeCharacter : public ACharacter, public IAbilitySystemInterface, public IMobaTeamInterface
{
	GENERATED_BODY()

public:
	AMobaDegreeCharacter();
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> AttackTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> OldAttackTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UTeamComponent> TeamComponent;

	

	virtual EGameTeam GetTeamInterface_Implementation() const override;

	//temp solution
	UFUNCTION()
	void MoveToLocation(FVector Location);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = _GAS)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = _GAS)
	TObjectPtr<UMobaAttributeSet> AttributeSet;

	void InitializeAttribute();

private:
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

public:
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};