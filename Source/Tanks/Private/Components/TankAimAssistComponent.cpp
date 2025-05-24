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

	// Get the tank's location and the target's location
	const FVector TankLocation = TankCharacter->GetActorLocation();
	
	// Calculate the direction vector from the tank to the target
	auto Look = UKismetMathLibrary::FindLookAtRotation(TankLocation, LockedTarget->GetActorLocation());
	
	// Handle horizontal aim assist (turret rotation)
	if (bEnableHorizontalAssist && HorizontalAssistStrength > 0.0f)
	{
		// Calculate the target direction
		auto TargetLocation = Look.Vector();
		TargetLocation.Z = 0.f;
		if (!TargetLocation.IsNearlyZero())
			TargetLocation.Normalize();

		FVector TurretForwardVector = TankCharacter->GetMesh()->GetSocketQuaternion("turret_jntSocket").GetForwardVector();
		TurretForwardVector.Z = 0.f;
		if (!TurretForwardVector.IsNearlyZero())
			TurretForwardVector.Normalize();

		// Calculate the target angle
		double DotProduct = FVector::DotProduct(TurretForwardVector, TargetLocation);

		// DO NOT CHANGE TOLERANCE (0.008 also works ig. idk which value is better)
		constexpr double Tolerance = 0.008; // setting it to 0.01 fixed it now somehow when it wasnt working before. DO NOT CHANGE
		if (FMath::IsNearlyEqual(DotProduct, 1.0f, Tolerance))
			DotProduct = 1.f; // Prevent any small rounding errors
		else if (FMath::IsNearlyEqual(DotProduct, -1.0f, Tolerance))
			DotProduct = -1.f; // Handle opposite direction

		double Det = FVector::CrossProduct(TurretForwardVector, TargetLocation).Z;
		if (FMath::IsNearlyZero(Det, Tolerance))
			Det = 0.f;

		double TargetAngle = FMath::RadiansToDegrees(FMath::Atan2(Det, DotProduct));
		if (FMath::IsNearlyEqual(TargetAngle, 1.0, Tolerance))
			TargetAngle = 1;

		// Calculate the *difference* in angle, but now wrap it to the shortest path
		double DeltaAngle = UKismetMathLibrary::NormalizeAxis(TargetAngle - TankCharacter->GetAnimInstance()->TurretAngle);

		// Clamp the angle difference based on MaxTurretRotationSpeed
		const double MaxDeltaAngle = TankCharacter->GetMaxTurretRotationSpeed() * GetWorld()->GetDeltaSeconds();
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);

		// Update the turret angle
		TankCharacter->SetTurretRotation(TankCharacter->GetAnimInstance()->TurretAngle + DeltaAngle);

		UKismetSystemLibrary::PrintString(
			  GetWorld(), 
			  FString::Printf(TEXT("(UTankAimAssistComponent::AimAssist) DeltaAngle: %.3f"), DeltaAngle), 
			  true, 
			  true, 
			  FLinearColor::Red, 
			  0
		);

		UKismetSystemLibrary::PrintString(
			  GetWorld(), 
			  FString::Printf(TEXT("(UTankAimAssistComponent::AimAssist) MaxDeltaAngle: %.3f"), MaxDeltaAngle), 
			  true, 
			  true, 
			  FLinearColor::Red, 
			  0
		);
	}

	FVector DirectionToTarget = Look.Vector();
	DirectionToTarget.Normalize();
	
	// Handle vertical aim assist (gun elevation)
	if (bEnableVerticalAssist && VerticalAssistStrength > 0.0f)
	{
		// Calculate the desired gun elevation angle
		// Note: We need to convert from world space to local space for elevation
		FVector LocalDirection = TankCharacter->GetActorTransform().InverseTransformVector(DirectionToTarget);
		double DesiredElevation = FMath::RadiansToDegrees(FMath::Atan2(LocalDirection.Z, FMath::Sqrt(LocalDirection.X * LocalDirection.X + LocalDirection.Y * LocalDirection.Y)));
		
		// Clamp the elevation within the tank's allowed limits
		DesiredElevation = FMath::Clamp(DesiredElevation, TankCharacter->GetAbsoluteMinGunElevation(), TankCharacter->GetAbsoluteMaxGunElevation());
		
		// Interpolate between current and desired elevation based on strength
		double NewGunElevation = FMath::Lerp(
			TankCharacter->GetGunElevation(), 
			DesiredElevation, 
			VerticalAssistStrength
		);
		
		// Apply the gun elevation speed limits (using GunElevationInterpSpeed from tank)
		const double ElevationDelta = NewGunElevation - TankCharacter->GetGunElevation();
		const double MaxElevationDelta = TankCharacter->GetWorld()->GetDeltaSeconds() * TankCharacter->GetGunElevationInterpSpeed();
		
		// Clamp the elevation change to respect the tank's elevation speed
		NewGunElevation = TankCharacter->GetGunElevation() + FMath::Clamp(ElevationDelta, -MaxElevationDelta, MaxElevationDelta);

		// Set the new gun elevation
		TankCharacter->SetGunElevation(Look.Pitch);
	}
}
