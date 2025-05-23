﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TankInterface.generated.h"

enum class EPowerUpType : uint8;
enum class ETeam : uint8;
// This class does not need to be modified.
UINTERFACE()
class UTankInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Main tank interface
 */
class TANKS_API ITankInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OutlineTank(const bool bActivate, const bool bIsFriend);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ETeam GetCurrentTeam();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PowerUpActivated(const EPowerUpType PowerUpType);
};
