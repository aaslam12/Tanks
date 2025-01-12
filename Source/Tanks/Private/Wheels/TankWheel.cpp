// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/Wheels/TankWheel.h"

#include "GameFramework/GameUserSettings.h"

UTankWheel::UTankWheel()
{
	auto UserSettings = UGameUserSettings::GetGameUserSettings();

	if (!UserSettings)
		return;

	// Value -1:custom, 0:low, 1:medium, 2:high, 3:epic, 4:cinematic
	auto ScalabilityLevel = UserSettings->GetOverallScalabilityLevel();

	switch (ScalabilityLevel)
	{
		case -1:
		{
			SweepShape = ESweepShape::Spherecast;
			break;
		}
		
		case 0:
		case 1:
		{
			SweepShape = ESweepShape::Raycast;
			break;
		}

		case 2:
		case 3:
		{
			SweepShape = ESweepShape::Spherecast;
			break;
		}
		
		case 4:
		{
			SweepShape = ESweepShape::Shapecast;
			break;
		}
	default: break;
	}
}
