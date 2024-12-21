// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleAnimationInstance.h"
#include "TankAnimInstance.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UTankAnimInstance : public UVehicleAnimationInstance
{
	GENERATED_BODY()

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	double WheelSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	double WheelSpeedOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	double TurretAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	double GunElevation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	double HatchAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator WheelRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator HatchRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator TurretRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator GunRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> TracksMID;

	UFUNCTION(BlueprintCallable)
	void UpdateSpeedOffset(const double Increment);
	
	UFUNCTION(BlueprintCallable)
	void UpdateWheels();

	UFUNCTION(BlueprintCallable)
	void UpdateHatches();

	UFUNCTION(BlueprintCallable)
	void UpdateTurret();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UpdateTracksMaterial();

	UFUNCTION(BlueprintCallable)
	void SetTracksMID(UMaterialInstanceDynamic* MID)
	{
		this->TracksMID = MID;
	}


};
