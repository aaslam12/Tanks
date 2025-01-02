// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankGameState.h"

#include "Net/UnrealNetwork.h"

void ATankGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Teams);
}

void ATankGameState::AssignPlayerToTeam(APlayerState* Player, const FString& TeamName)
{
	if (!Player) return;

	// Search for the team by name
	FTeamData* Team = Teams.FindByPredicate([&](const FTeamData& Data) {
	  return Data.TeamName == TeamName;
	});

	if (Team)
	{
		// Add player to the existing team
		Team->Players.Add(Player);
	}
	else
	{
		// Create a new team if not found
		FTeamData NewTeam;
		NewTeam.TeamName = TeamName;
		NewTeam.Players.Add(Player);
		Teams.Add(NewTeam);
	}
}

TArray<APlayerState*> ATankGameState::GetPlayersInTeam(const FString& TeamName) const
{
	const FTeamData* Team = Teams.FindByPredicate([&](const FTeamData& Data) {
	  return Data.TeamName == TeamName;
	});

	return Team ? Team->Players : TArray<APlayerState*>();
}