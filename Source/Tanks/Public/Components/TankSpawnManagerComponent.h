// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Libraries/TankEnumLibrary.h"
#include "TankSpawnManagerComponent.generated.h"

enum class ETeam : uint8;

USTRUCT(BlueprintType)
struct FTeamSpawn
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ETeam TeamName;

	UPROPERTY(BlueprintReadOnly)
	TArray<APlayerStart*> TeamSpawnPoints;

	FTeamSpawn(): TeamName(ETeam::Team1)
	{
	}

	FTeamSpawn(const ETeam TeamName, const TArray<APlayerStart*>& TeamSpawnPoints)
		: TeamName(TeamName),
		  TeamSpawnPoints(TeamSpawnPoints)
	{
	}
};

/**
 * Handles player spawns. Only exists on the SERVER.
 */
UCLASS(Blueprintable)
class TANKS_API UTankSpawnManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	UTankSpawnManagerComponent();

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TArray<FTeamSpawn> SpawnPoints;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TArray<FTeamSpawn> AvailableSpawnPoints;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TArray<FTeamSpawn> UnavailableSpawnPoints;
	
	bool bDefaultsSet;
	
public:
	void MakePlayerStartUnavailable(APlayerStart* PlayerStart, ETeam Team);
	void SetDefaults();
	bool IsPlayerStartAvailable(APlayerStart* PlayerStart);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetTeamSpawnIndex(const ETeam Team);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlayerStart* GetSpawnLocation(const ETeam Team);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetRandomSpawnPointFromTeam(ETeam Team);
};
