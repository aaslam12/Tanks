// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankAimAssistComponent.h"

#include "TankCharacter.h"
#include "Animation/TankAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

void UTankAimAssistComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		if (Cast<ATankCharacter>(GetOwner()))
			TankCharacter = Cast<ATankCharacter>(GetOwner());
}

// Sets default values for this component's properties
UTankAimAssistComponent::UTankAimAssistComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UTankAimAssistComponent::AimAssist(AActor* const LockedTarget) const
{
	if (!TankCharacter || !LockedTarget || !bEnableAimAssist)
		return;

	if (!bEnableHorizontalAssist && !bEnableVerticalAssist)
		return;

	UWorld* WorldContextObject = GetWorld();

	// Get the tank's location and the target's location
	const FVector TankLocation = TankCharacter->GetActorLocation();
	
	// Calculate the direction vector from the tank to the target
	auto Look = UKismetMathLibrary::FindLookAtRotation(TankLocation, LockedTarget->GetActorLocation());
	auto LookVector = Look.Vector();
	
	// Handle horizontal aim assist (turret rotation)
	if (bEnableHorizontalAssist && HorizontalAssistStrength > 0.0f)
	{
		// Calculate the target direction
		auto TargetLocation = LookVector;
		TargetLocation.Z = 0.f;

		FVector TurretForwardVector = TankCharacter->GetMesh()->GetSocketQuaternion(FName("TurretSocket")).GetForwardVector();
		TurretForwardVector.Z = 0.f;
		if (!TurretForwardVector.IsNearlyZero())
			TurretForwardVector.Normalize();

		// Calculate the target angle
		float DotProduct = FMath::Clamp(
			FVector::DotProduct(TurretForwardVector, TargetLocation),
			-1, 1
		);

		// DO NOT CHANGE TOLERANCE (0.008 also works ig. idk which value is better)
		constexpr double Tolerance = 0.008; // setting it to 0.01 fixed it now somehow when it wasn't working before. DO NOT CHANGE
		if (FMath::IsNearlyEqual(DotProduct, 1.0f, Tolerance))
			DotProduct = 1.f; // Prevent any small rounding errors
		else if (FMath::IsNearlyEqual(DotProduct, -1.0f, Tolerance))
			DotProduct = -1.f; // Handle opposite direction

		float Det = FVector::CrossProduct(TurretForwardVector, TargetLocation).Z;
		if (FMath::IsNearlyZero(Det, Tolerance))
			Det = 0.f;

		float TargetAngle = FMath::RadiansToDegrees(FMath::Atan2(Det, DotProduct));
		if (FMath::IsNearlyEqual(TargetAngle, 1.0, Tolerance))
			TargetAngle = 1;

		const auto TankTurretAngle = TankCharacter->GetAnimInstance()->TurretAngle;

		// Clamp the angle difference based on MaxTurretRotationSpeed
		const float MaxDeltaAngle = TankCharacter->GetMaxTurretRotationSpeed() * WorldContextObject->GetDeltaSeconds();

		// Calculate the *difference* in angle, but now wrap it to the shortest path
		float DeltaAngle = FMath::Clamp(
			TargetAngle - TankTurretAngle,
			-MaxDeltaAngle,
			MaxDeltaAngle
		);

		// Update the turret angle
		TankCharacter->SetTurretRotation(TankTurretAngle + DeltaAngle);
	}

	// FVector DirectionToTarget = LookVector;
	// DirectionToTarget.Normalize();
	
	// Handle vertical aim assist (gun elevation)
	if (bEnableVerticalAssist && VerticalAssistStrength > 0.0f)
	{
		// Set the new gun elevation
		TankCharacter->SetGunElevation(Look.Pitch);
	}
}
