// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankController.generated.h"

class ATankCameraManager;
/**
 * The base class for the tank controllers
 */
UCLASS(Blueprintable)
class TANKS_API ATankController : public APlayerController
{
	GENERATED_BODY()

	ATankController();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATankCameraManager> TankCameraManagerClass;

	
};
