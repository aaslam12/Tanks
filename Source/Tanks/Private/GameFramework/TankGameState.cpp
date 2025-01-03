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
	// for (FTeamData& Team : Teams)
	// {
	// 	Team.Players.Remove(nullptr);  // Clean up any null pointers
	// }
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankGameState::OnRep_Teams) Teams updated")),
			true, true, FLinearColor::Yellow, 15);

	for (const FTeamData& Team : Teams)
	{
		UE_LOG(LogTemp, Log, TEXT("Team: %s"), *Team.TeamName);
		for (auto Player : Team.Players)
		{
			if (Player)
			{
				UE_LOG(LogTemp, Log, TEXT("  Player: %s"), *Player->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("  Player: nullptr"));
			}
		}
	}
}

void ATankGameState::OnPostLogin(APlayerState* NewPlayer)
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
		if (Team.TeamName.Equals("Team 1"))
			Team1Count += Team.Players.Num();
		else if (Team.TeamName.Equals("Team 2"))
			Team2Count += Team.Players.Num();
	}

	// Assign to the team with fewer players
	FString TeamToAssign = Team1Count <= Team2Count ? "Team 1" : "Team 2";

	// Assign the player to the chosen team
	AssignPlayerToTeam(NewPlayer, TeamToAssign);

	// Update the player's name to reflect their team assignment
	auto PlayerName = FString::Printf(TEXT("Player %d (%s) [%s]"), NewPlayer->GetPlayerId(), *TeamToAssign, *NewPlayer->GetName());
	NewPlayer->SetPlayerName(PlayerName);
}

void ATankGameState::AssignPlayerToTeam(APlayerState* Player, const FString& TeamName)
{
	if (!Player) return;

	// Search for the team by name
	auto Team = FindTeamByName(TeamName);

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

FTeamData* ATankGameState::FindTeamByName(const FString& TeamName)
{
	FTeamData* Team = Teams.FindByPredicate([&](const FTeamData& Data)
	{
		return Data.TeamName.Equals(TeamName);
	});

	return Team ? Team : nullptr;
}

const FTeamData& ATankGameState::GetPlayersInTeam(const FString& TeamName)
{
	FTeamData* Team = Teams.FindByPredicate([&](const FTeamData& Data) {
		return Data.TeamName.Equals(TeamName, ESearchCase::IgnoreCase);
	});

	auto e = FTeamData();

	return Team ? *Team : e;
}