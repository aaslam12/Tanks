// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/Animation/TankAnimInstance.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void UTankAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateSpeedOffset(DeltaSeconds);
	SR_UpdateWheels();
	SR_UpdateHatches();
	SR_UpdateTurret();
	SR_UpdateTracksMaterial();
}

void UTankAnimInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTankAnimInstance, WheelSpeed);
	DOREPLIFETIME(UTankAnimInstance, WheelSpeedOffset);
	DOREPLIFETIME(UTankAnimInstance, TurretAngle);
	DOREPLIFETIME(UTankAnimInstance, GunElevation);
	DOREPLIFETIME(UTankAnimInstance, HatchAngle);
	DOREPLIFETIME(UTankAnimInstance, WheelRotation);
	DOREPLIFETIME(UTankAnimInstance, HatchRotation);
	DOREPLIFETIME(UTankAnimInstance, TurretRotation);
	DOREPLIFETIME(UTankAnimInstance, GunRotation);
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

void UTankAnimInstance::MC_SetGunElevation_Implementation(const double NewGunElevation)
{
	GunElevation = NewGunElevation;
}

void UTankAnimInstance::SR_SetGunElevation_Implementation(const double NewGunElevation)
{
	MC_SetGunElevation(NewGunElevation);
}

void UTankAnimInstance::MC_UpdateTracksMaterial_Implementation()
{
	UpdateTracksMaterial();
}

void UTankAnimInstance::MC_UpdateWheels_Implementation()
{
	UpdateWheels();
}

void UTankAnimInstance::MC_UpdateHatches_Implementation()
{
	UpdateHatches();
}

void UTankAnimInstance::MC_UpdateTurret_Implementation()
{
	UpdateTurret();
}

void UTankAnimInstance::SR_UpdateWheels_Implementation()
{
	MC_UpdateWheels();
}

void UTankAnimInstance::SR_UpdateHatches_Implementation()
{
	MC_UpdateHatches();
}

void UTankAnimInstance::SR_UpdateTurret_Implementation()
{
	MC_UpdateTurret();
}

void UTankAnimInstance::SR_UpdateTracksMaterial_Implementation()
{
	MC_UpdateTracksMaterial();
}
