// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/Wheels/TankWheel.h"

UTankWheel::UTankWheel()
{
	///////////////////////////////////////////////////
	/// Wheel
	Offset = FVector(0, 0, -15);
	WheelRadius = 32.0;
	WheelWidth = 40.0;
	WheelMass = 500.0;
	CorneringStiffness = 1000.0;
	SideSlipModifier = 1.0;
	SlipThreshold = 0.0;
	SkidThreshold = 0.0;
	bAffectedByBrake = true;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
	bABSEnabled = true;
	
	///////////////////////////////////////////////////
	/// Suspension
	SuspensionMaxRaise = 10.0;
	SuspensionMaxDrop = 10.0;
	SuspensionDampingRatio = 0.5;
	WheelLoadRatio = 0.5;
	SpringRate = 250.0;
	SuspensionSmoothing = 5;
	RollbarScaling = 0.15;
	SweepShape = ESweepShape::Spherecast;

	///////////////////////////////////////////////////
	/// Brakes
	MaxBrakeTorque = 1500.0;
	MaxHandBrakeTorque = 4000.0;
}
