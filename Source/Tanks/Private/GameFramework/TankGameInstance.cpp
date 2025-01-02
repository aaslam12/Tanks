// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankGameInstance.h"

void UTankGameInstance::AssignPlayerToTeam(APlayerState* Player, const FString& TeamName)
{
	if (!Player) return;

	// Add player to the team
	if (Teams.Contains(TeamName))
	{
		Teams[TeamName].Players.Add(Player);
	}
	else
	{
		FTeamData NewTeam;
		NewTeam.Players.Add(Player);
		Teams.Add(TeamName, NewTeam);
	}
}

TArray<APlayerState*> UTankGameInstance::GetPlayersInTeam(const FString& TeamName) const
{
	const FTeamData* TeamData = Teams.Find(TeamName);
	return TeamData ? TeamData->Players : TArray<APlayerState*>();
}