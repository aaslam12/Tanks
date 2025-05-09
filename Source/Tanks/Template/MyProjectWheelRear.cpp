// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UMyProjectWheelRear::UMyProjectWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}