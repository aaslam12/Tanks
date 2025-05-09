// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UMyProjectWheelFront::UMyProjectWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}