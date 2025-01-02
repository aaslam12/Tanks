// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Libraries/TankStructLibrary.h"
#include "TankGameState.generated.h"

struct FTeamData;
/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API ATankGameState : public AGameState
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Teams")
	TArray<FTeamData> Teams;
};
