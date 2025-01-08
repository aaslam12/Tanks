// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankSpawnManagerComponent.h"

#include "EnhancedCodeFlow.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/TankEnumLibrary.h"


// Sets default values for this component's properties
UTankSpawnManagerComponent::UTankSpawnManagerComponent(): bDefaultsSet(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UTankSpawnManagerComponent::MakePlayerStartUnavailable(APlayerStart* PlayerStart, ETeam Team)
{
	auto UnavailableTeamSpawn = UnavailableSpawnPoints.FindByPredicate([&](const FTeamSpawn& TeamSpawn)
	{
		return TeamSpawn.TeamName == Team;
	});
	
	if (UnavailableTeamSpawn)
	{
		// add it to array if it doesn't already exist in it
		if (!UnavailableTeamSpawn->TeamSpawnPoints.Contains(PlayerStart))
			UnavailableTeamSpawn->TeamSpawnPoints.Add(PlayerStart);
	}
	else
	{
		FTeamSpawn TeamSpawn;
		TeamSpawn.TeamName = Team;
		TeamSpawn.TeamSpawnPoints.Add(PlayerStart);
		UnavailableSpawnPoints.Add(TeamSpawn);

		for (int i = 0; i < AvailableSpawnPoints.Num(); ++i)
		{
			if (AvailableSpawnPoints[i].TeamSpawnPoints.Contains(PlayerStart))
			{
				AvailableSpawnPoints[i].TeamSpawnPoints.Remove(PlayerStart);
				return;
			}
		}
	}

	FFlow::Delay(GetWorld(), 5, [Team, PlayerStart, UnavailableTeamSpawn, this]
	{
		auto AvailableTeamSpawn = AvailableSpawnPoints.FindByPredicate([&](const FTeamSpawn& TeamSpawn)
		{
			return TeamSpawn.TeamName == Team;
		});

		if (AvailableTeamSpawn)
		{
			if (!AvailableTeamSpawn->TeamSpawnPoints.Contains(PlayerStart))
				AvailableTeamSpawn->TeamSpawnPoints.Add(PlayerStart);
		}
		else
		{
			FTeamSpawn TeamSpawn;
			TeamSpawn.TeamName = Team;
			TeamSpawn.TeamSpawnPoints.Add(PlayerStart);
			AvailableSpawnPoints.Add(TeamSpawn);
		}
		
		UnavailableTeamSpawn->TeamSpawnPoints.Remove(PlayerStart);
	});
}

void UTankSpawnManagerComponent::SetDefaults()
{
	// if (bDefaultsSet == true)
	// 	return;
	
	// get all player start points in the map
	TArray<AActor*> PlayerStartPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartPoints);
	const TArray<ETeam> Teams = CreateTeamArray();

	// loop through them all once to get each team's spawn points
	for (const auto StartPoint : PlayerStartPoints)
	{
		auto PlayerStart = Cast<APlayerStart>(StartPoint);
		if (!PlayerStart)
			continue;
		
		for (const ETeam& Team : Teams)
		{
			FString TeamTag = UEnum::GetValueAsString(Team); // Converts enum to string like "ETeam::Team1"
			TeamTag.RemoveFromStart("ETeam::");

			// Filter spawn points for the current team
			if (PlayerStart && PlayerStart->PlayerStartTag.IsEqual(FName(*TeamTag)))
			{
				auto ElementIndex = GetTeamSpawnIndex(Team);

				if (ElementIndex == INDEX_NONE)
				{
					// if team does not exist, create one.
					FTeamSpawn TeamSpawn;
					TeamSpawn.TeamName = Team;
					TeamSpawn.TeamSpawnPoints.Add(PlayerStart);
					SpawnPoints.Add(TeamSpawn);
					AvailableSpawnPoints.Add(TeamSpawn);
					break; // stop it from running for other teams
				}
				else
				{
					// if team exists, add this player point to array
					SpawnPoints[ElementIndex].TeamSpawnPoints.Add(PlayerStart);
					AvailableSpawnPoints[ElementIndex].TeamSpawnPoints.Add(PlayerStart);
					break; // stop it from running for other teams
				}
			}
			else
			{
				if (!StartPoint)
					UE_LOG(LogTemp, Error, TEXT("StartPoint is null"));
			}
		}
	}

	// bDefaultsSet = false;
}

bool UTankSpawnManagerComponent::IsPlayerStartAvailable(APlayerStart* PlayerStart)
{
	for (auto Element : AvailableSpawnPoints)
		if (Element.TeamSpawnPoints.Contains(PlayerStart))
			return true;

	return false;
}

int32 UTankSpawnManagerComponent::GetTeamSpawnIndex(const ETeam Team)
{
	for (int i = 0; i < SpawnPoints.Num(); ++i)
		if (SpawnPoints[i].TeamName == Team)
			return i;
	
	return -1;
}

APlayerStart* UTankSpawnManagerComponent::GetSpawnLocation(const ETeam Team)
 {
	auto Spawns = GetTeamSpawnIndex(Team);
	if (Spawns == INDEX_NONE)
		return nullptr;

	// if the game is not FFA, spawn in team base. or else spawn anywhere on random.
	if (Team != ETeam::NoTeam)
		for (auto Element : SpawnPoints[Spawns].TeamSpawnPoints)
			if (IsPlayerStartAvailable(Element))
				return Element; // this means that there is no unavailable spawn point. all of our team spawn points are free

	// means no spawn point is available so choose a random one anyway.
	UE_LOG(LogTemp, Error, TEXT("Spawning at a random location..."));
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Spawning at a random location...")),
		true, true, FLinearColor::Yellow, 15);
	
	return SpawnPoints[Spawns].TeamSpawnPoints[FMath::RandRange(0, SpawnPoints[Spawns].TeamSpawnPoints.Num() - 1)]; 
}
