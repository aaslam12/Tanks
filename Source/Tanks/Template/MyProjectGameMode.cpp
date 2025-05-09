// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectGameMode.h"
#include "MyProjectPlayerController.h"

AMyProjectGameMode::AMyProjectGameMode()
{
	PlayerControllerClass = AMyProjectPlayerController::StaticClass();
}
