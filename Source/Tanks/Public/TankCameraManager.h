// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "TankCameraManager.generated.h"

/**
 * The base class for the tank camera managers
 */
UCLASS(Blueprintable)
class TANKS_API ATankCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	ATankCameraManager();

	// tick function
	virtual void UpdateCamera(float DeltaTime) override;
};
