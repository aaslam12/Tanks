// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankGameState.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/TankPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

void ATankGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Teams);
}

void ATankGameState::OnRep_Teams()
{
}

void ATankGameState::AssignPlayerToTeam(APlayerState* NewPlayer)
{
	if (!UKismetSystemLibrary::IsValid(NewPlayer))
		return;

	// Count the number of players in each team
	int32 Team1Count = 0;
	int32 Team2Count = 0;

	for (const FTeamData& Team : Teams)
	{
		// Suggestion: Each player can have a "score" that is based on how strong their tank is.
		// This can then be used to place a player in an appropriate team. 
		if (Team.TeamName == ETeam::Team1)
			Team1Count += Team.Players.Num();
		else if (Team.TeamName == ETeam::Team2)
			Team2Count += Team.Players.Num();
	}

	// Assign to the team with fewer players
	ETeam TeamToAssign = Team1Count <= Team2Count ? ETeam::Team1 : ETeam::Team2;
	
	// Assign the player to the chosen team
	AssignPlayerToTeam(NewPlayer, TeamToAssign);

	// Update the player's name to reflect their team assignment
	auto PlayerName = FString::Printf(TEXT("Player %d (%d) [%s]"), NewPlayer->GetPlayerId(), (int)TeamToAssign, *NewPlayer->GetName());
	NewPlayer->SetPlayerName(PlayerName);
}

void ATankGameState::AssignPlayerToTeam(APlayerState* Player, const ETeam TeamName)
{
	if (!Player) return;

	// Search for the team by name
	auto Team = FindTeam(TeamName);

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

	if (auto e = Cast<ATankPlayerState>(Player))
		e->SetCurrentTeam(TeamName);
}

FTeamData* ATankGameState::FindTeam(const ETeam TeamName)
{
	FTeamData* Team = Teams.FindByPredicate([&](const FTeamData& Data)
	{
		return Data.TeamName == TeamName;
	});

	return Team ? Team : nullptr;
}

const FTeamData& ATankGameState::GetPlayersInTeam(const ETeam TeamName)
{
	FTeamData* Team = Teams.FindByPredicate([&](const FTeamData& Data) {
		return Data.TeamName == TeamName;
	});

	auto e = FTeamData();

	return Team ? *Team : e;
}