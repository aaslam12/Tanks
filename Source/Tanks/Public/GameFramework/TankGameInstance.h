// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Libraries/TankStructLibrary.h"
#include "TankGameInstance.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UTankGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category="Teams")
	TArray<FString> TeamNames;

	UPROPERTY(BlueprintReadWrite, Category="Teams")
	TMap<FString, FTeamData> Teams; // Key: Team Name, Value: List of Players

	UFUNCTION(BlueprintCallable, Category="Teams")
	void AssignPlayerToTeam(APlayerState* Player, const FString& TeamName);

	UFUNCTION(BlueprintCallable, Category="Teams")
	TArray<APlayerState*> GetPlayersInTeam(const FString& TeamName) const;

	/**
	 * Called from the Game mode only.
	 * @param NewPlayer The newly logged on player
	 */
	virtual void OnPostLogin(AController* NewPlayer);
};
