// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "TankWheel.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UTankWheel : public UChaosVehicleWheel
{
	GENERATED_BODY()

public:
	UTankWheel();
	void SetSweepShapeBasedOnGraphicsSettings();
};
