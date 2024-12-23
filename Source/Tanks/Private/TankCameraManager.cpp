// Fill out your copyright notice in the Description page of Project Settings.


#include "TankCameraManager.h"

#include "Kismet/KismetSystemLibrary.h"

ATankCameraManager::ATankCameraManager()
{

	// if (GetWorld())
	// 	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCameraManager::ATankCameraManager) Constructor called")),
	// 		true, true, FLinearColor::Yellow, 0);

}

void ATankCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCameraManager::UpdateCamera)")),
	// 	true, true, FLinearColor::Yellow, 0);
}
