// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankSpawnManagerComponent.generated.h"

enum class ETeam : uint8;

USTRUCT(BlueprintType)
struct FTeamSpawn
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ETeam TeamName;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> TeamSpawnPoints;
};

/**
 * Handles player spawns. Only exists on the SERVER.
 */
UCLASS(Abstract, ClassGroup=(Custom))
class TANKS_API UTankSpawnManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	UTankSpawnManagerComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly)
	TArray<FTeamSpawn> SpawnPoints;
};
