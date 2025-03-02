#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Team/EGameTeam.h"
#include "Team/Interface/TeamInterface.h"
#include "TeamComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBADEGREE_API UTeamComponent : public UActorComponent, public ITeamInterface
{
	GENERATED_BODY()

public:	
	UTeamComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual EGameTeam GetTeamInterface() override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditInstanceOnly)
	EGameTeam Team{ EGameTeam::None };

public:
	UFUNCTION(BlueprintCallable, Category = "Team")
	FORCEINLINE	void SetTeam(EGameTeam TeamToChose) { Team = TeamToChose; }

	UFUNCTION(BlueprintCallable, Category = "Team")
	FORCEINLINE	EGameTeam GetTeam() { return Team; }
	
};
