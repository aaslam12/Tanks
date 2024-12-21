// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/Wheels/TankWheel.h"

UTankWheel::UTankWheel()
{
	///////////////////////////////////////////////////
	/// Wheel
	WheelWidth = 40.0;
	WheelMass = 500.0;
	SlipThreshold = 1000.0;
	SkidThreshold = 1000.0;
	bAffectedByBrake = true;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
	bABSEnabled = true;

	///////////////////////////////////////////////////
	/// Suspension
	SuspensionMaxRaise = 5.0;
	SuspensionMaxDrop = 5.0;
	SpringRate = 5000.0;
	SuspensionSmoothing = 1;
}
