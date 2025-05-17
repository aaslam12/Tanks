// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/Wheels/TankWheel.h"

#include "GameFramework/GameUserSettings.h"

UTankWheel::UTankWheel()
{
    SetSweepShapeBasedOnGraphicsSettings();
}

void UTankWheel::SetSweepShapeBasedOnGraphicsSettings()
{
    if (!GEngine)
        return;

    auto UserSettings = UGameUserSettings::GetGameUserSettings();
    if (!UserSettings)
        return;
    
    int ScalabilityLevel = UserSettings->GetOverallScalabilityLevel();
    UE_LOG(LogTemp, Log, TEXT("ScalabilityLevel: %d"), ScalabilityLevel);
    
    // Value -1:custom, 0:low, 1:medium, 2:high, 3:epic, 4:cinemati
    SweepShape = (ScalabilityLevel <= 1) ? ESweepShape::Raycast : ESweepShape::Spherecast;
}
