// Fill out your copyright notice in the Description page of Project Settings.


#include "TankController.h"

#include "TankCameraManager.h"

ATankController::ATankController()
{
	if (TankCameraManagerClass)
		PlayerCameraManagerClass = TankCameraManagerClass;
}
