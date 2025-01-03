// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Libraries/TankStructLibrary.h"
#include "TankGameState.generated.h"

struct FTeamData;
/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API ATankGameState : public AGameState
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Teams();
	
public:
	virtual void OnPostLogin(APlayerState* NewPlayer);
	
	FTeamData* FindTeamByName(const FString& TeamName);

	UPROPERTY(ReplicatedUsing=OnRep_Teams, BlueprintReadOnly, Category="Teams")
	TArray<FTeamData> Teams;
	
	// Assign a player to a team
	UFUNCTION(BlueprintCallable, Category="Teams")
	void AssignPlayerToTeam(APlayerState* Player, const FString& TeamName);

	// Get players in a specific team
	UFUNCTION(BlueprintCallable, Category="Teams")
	const FTeamData& GetPlayersInTeam(const FString& TeamName);
};
