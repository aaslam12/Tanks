// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Libraries/TankEnumLibrary.h"
#include "TankStructLibrary.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTeamData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Team Data")
	ETeam TeamName;
	
	UPROPERTY(BlueprintReadWrite, Category="Team Data")
	TArray<APlayerState*> Players;

	FTeamData(): TeamName(ETeam::Unassigned)
	{
	}
};
