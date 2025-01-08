#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Unassigned UMETA(Hidden),
	Team1 UMETA(DisplayName = "Team1"),
	Team2 UMETA(DisplayName = "Team2"),

	NoTeam UMETA(DisplayName = "NoTeam"),
};

/**
 * Creates an array with every single possible value of its corresponding enum.
 */
FORCEINLINE TArray<ETeam> CreateTeamArray()
{
	TArray<ETeam> Teams;
	Teams.Add(ETeam::Team1);
	Teams.Add(ETeam::Team2);
	Teams.Add(ETeam::NoTeam);
	return Teams;
}