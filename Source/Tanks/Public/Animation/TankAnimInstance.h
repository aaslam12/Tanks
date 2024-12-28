// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleAnimationInstance.h"
#include "TankAnimInstance.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TANKS_API UTankAnimInstance : public UVehicleAnimationInstance
{
	GENERATED_BODY()

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", Replicated)
	double WheelSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup")
	double WheelSpeedOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", Replicated)
	double TurretAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", Replicated)
	double GunElevation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", Replicated)
	double HatchAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FRotator WheelRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FRotator HatchRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FRotator TurretRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
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
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_UpdateWheels();
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_UpdateHatches();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_UpdateTurret();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_UpdateTracksMaterial();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_UpdateWheels();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_UpdateHatches();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_UpdateTurret();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_UpdateTracksMaterial();

	UFUNCTION(BlueprintCallable)
	void SetTracksMID(UMaterialInstanceDynamic* MID) { this->TracksMID = MID; }

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_SetGunElevation(const double NewGunElevation);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetGunElevation(const double NewGunElevation);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	double GetGunElevation() { return GunElevation; }

};
