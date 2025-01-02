// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankGameInstance.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Libraries/TankStructLibrary.h"

void UTankGameInstance::AssignPlayerToTeam(APlayerState* Player, const FString& TeamName)
{
	if (!Player) return;

	// Add player to the team
	if (Teams.Contains(TeamName))
	{
		Teams[TeamName].Players.Add(Player);
		Teams[TeamName].TeamName = TeamName;
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

void UTankGameInstance::OnPostLogin(AController* NewPlayer)
{
	if (!UKismetSystemLibrary::IsValid(NewPlayer))
		return;

	APlayerState* PlayerState = NewPlayer->PlayerState;
	if (!PlayerState) return;

	// Count the number of players in each team
	int32 Team1Count = 0;
	int32 Team2Count = 0;

	for (const TTuple<FString, FTeamData>& Team : Teams)
	{
		// Suggestion: Each player can have a "score" that is based on how strong their tank is.
		// This can then be used to place a player in an appropriate team. 
		if (Team.Key == "Team 1")
		{
			Team1Count += Team.Value.Players.Num();
		}
		else if (Team.Key == "Team 2")
		{
			Team2Count += Team.Value.Players.Num();
		}
	}

	// Assign to the team with fewer players
	FString TeamToAssign = (Team1Count <= Team2Count) ? "Team 1" : "Team 2";

	// Assign the player to the chosen team
	AssignPlayerToTeam(PlayerState, TeamToAssign);

	// Update the player's name to reflect their team assignment
	auto PlayerName = FString::Printf(TEXT("Player %d (%s) [%s]"), PlayerState->GetPlayerId(), *TeamToAssign, *NewPlayer->GetName());
	PlayerState->SetPlayerName(PlayerName);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankGameInstance::OnPostLogin) PlayerName: %s"), *PlayerName),
			true, true, FLinearColor::Yellow, 15);
}
