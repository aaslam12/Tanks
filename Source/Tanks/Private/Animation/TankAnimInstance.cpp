// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/Animation/TankAnimInstance.h"

#include "Kismet/KismetMathLibrary.h"

void UTankAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateSpeedOffset(DeltaSeconds);
	UpdateWheels();
	UpdateHatches();
	UpdateTurret();
	UpdateTracksMaterial();
}

void UTankAnimInstance::UpdateSpeedOffset(const double Increment)
{
	WheelSpeedOffset = UKismetMathLibrary::GenericPercent_FloatFloat(Increment + WheelSpeedOffset, 360.0);
}

void UTankAnimInstance::UpdateWheels()
{
	WheelRotation = FRotator(WheelSpeedOffset * WheelSpeed * -1.0, 0, 0);
}

void UTankAnimInstance::UpdateHatches()
{
	HatchRotation = FRotator(HatchAngle, 0, 0);
}

void UTankAnimInstance::UpdateTurret()
{
	TurretRotation = FRotator(0, TurretAngle, 0);
	GunRotation = FRotator(GunElevation, 0, 0);
}

