#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Team_1 UMETA(DisplayName = "Team 1"),
	Team_2 UMETA(DisplayName = "Team 2"),
};

USTRUCT(BlueprintType)
struct FTeam
{
	GENERATED_BODY()

	TArray<ETeam> Teams;

	FTeam()
	{
		Teams.Add(ETeam::Team_1);
		Teams.Add(ETeam::Team_2);
	}
};