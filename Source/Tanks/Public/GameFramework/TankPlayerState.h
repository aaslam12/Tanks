// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TankPlayerState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API ATankPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, BlueprintReadWrite, Category="Teams")
	FString TeamName;

	UFUNCTION(BlueprintCallable, Category="Teams")
	FString GetTeamName() const { return TeamName; }

	UFUNCTION(BlueprintCallable, Category="Teams")
	void SetTeamName(const FString& NewTeamName) { TeamName = NewTeamName; }
};
