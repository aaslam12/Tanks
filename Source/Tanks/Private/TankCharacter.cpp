// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "TankController.h"
#include "Camera/CameraComponent.h"
#include "Components/TankHealthComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"


void ATankCharacter::InitializeHealthComponent()
{
	// if (!TankHealthComponentClass)
	// {
	// 	// create the pure c++ version if bp version is not specified
	// 	HealthComponent = CreateDefaultSubobject<UTankHealthComponent>("HealthComponent");
	// }
	// else
	// {
	// 	// create the bp version of the component
	// 	HealthComponent = Cast<UTankHealthComponent>(CreateDefaultSubobject("HealthComponent",
	// 	                                                                    TankHealthComponentClass, TankHealthComponentClass,
	// 	                                                                    true, false));
	// }
}

// Sets default values
ATankCharacter::ATankCharacter(): MaxZoomIn(500), MaxZoomOut(2500), BasePitchMin(-20.0), BasePitchMax(10.0),
                                  AbsoluteMinGunElevation(-5), AbsoluteMaxGunElevation(30), MaxTurretRotationSpeed(90),
                                  GunElevationInterpSpeed(10),
                                  MinGunElevation(-15), MaxGunElevation(20), CurrentTurretAngle(0), GunElevation(0),
                                  bIsInAir(false), 
                                  DesiredGunElevation(0), LineTraceOffset(0),
                                  LineTraceForwardVectorMultiplier(8000), VerticalLineTraceHalfSize(FVector(10, 10, 300)), HorizontalLineTraceHalfSize(FVector(10, 300, 10)),
                                  LookValues(), MoveValues(), bAimingIn(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	InitializeHealthComponent();

	if (GetMesh())
	{
		// Enable custom depth for the tank mesh
		GetMesh()->SetRenderCustomDepth(true);
		// Set the custom depth stencil value to differentiate between different types of objects
		GetMesh()->SetCustomDepthStencilValue(0);
	}

	for (auto Element : GetComponents())
	{
		// if (Element->StaticClass() == UStaticMeshComponent::StaticClass() || Element->StaticClass() == USkeletalMeshComponent::StaticClass())
		Element->SetIsReplicated(true);
	}
}

ATankCharacter::~ATankCharacter() {}

void ATankCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ShootSocket = GetShootSocke();

	FrontCameraComp = GetFrontCamera();
	FrontSpringArmComp = GetFrontSpringArm();

	BackCameraComp = GetBackCamera();
	BackSpringArmComp = GetBackSpringArm();
}

void ATankCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentTurretAngle);
}

// Called every frame
void ATankCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld())
		return;

	// Only run trace logic for the local player or server, not unnecessary clients
	if (HasAuthority() || IsLocallyControlled())
	{
		// If we're the server or the local player, run the traces and related logic
		if (PlayerController)
		{
			MoveValues = PlayerController->GetMoveValues();
			LookValues = PlayerController->GetLookValues();
		}

		// Execute other logic
		TurretTurningTick(DeltaTime);
		GunElevationTick(DeltaTime);
		CheckIfGunCanLowerElevationTick(DeltaTime);
		GunSightTick(GunTraceEndpoint, GunTraceScreenPosition);

		IsInAirTick();
		HighlightEnemyTanksIfDetected();

		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Tick) Tick running")),
			true, true, FLinearColor::Yellow, 0);
	}
}

void ATankCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// cache the EnhancedInputComponent to bind controls later in BeginPlay after the controller is available.
	if (PlayerInputComponent)
		if (Cast<UEnhancedInputComponent>(PlayerInputComponent))
			EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

void ATankCharacter::SetDefaults()
{
	SetActorScale3D(FVector(0.95));
	AnimInstance = Cast<UTankAnimInstance>(GetMesh()->GetAnimInstance());

	PlayerController = Cast<ATankController>(GetController());
	// PlayerController->Possess(this);

	// stops the player from looking under the tank and above too much.
	if (PlayerController)
	{
		PlayerController->PlayerCameraManager->ViewPitchMin = -60.0;
		PlayerController->PlayerCameraManager->ViewPitchMax = 30.0;
	}

	if (FrontCameraComp)
		FrontCameraComp->SetActive(false);

	if (BackCameraComp)
		BackCameraComp->SetActive(true);

	SetLightsEmissivity(0);
}

void ATankCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetDefaults();
}

void ATankCharacter::SR_TurretTurningTick_Implementation(const bool bAimingIn_, const float TurretAngle_, const FVector2D& LookValues_, const double MaxTurretRotationSpeed_)
{
	if (bAimingIn_)
	{
		// first person turret rotation
		float DesiredTurretAngle = TurretAngle_ + FMath::Clamp(LookValues_.X * 25, -MaxTurretRotationSpeed_ / 2, MaxTurretRotationSpeed_ / 2);

		// Smoothly interpolate towards the desired angle
		CurrentTurretAngle = FMath::FInterpTo(
			CurrentTurretAngle, // Current value
			DesiredTurretAngle, // Target value
			GetWorld()->GetDeltaSeconds(), // Time delta
			3.0f // Interpolation speed (adjust for more lag or responsiveness)
		);

		// Apply the interpolated value to the turret rotation
		MC_SetTurretRotation(CurrentTurretAngle);
	}
	else
	{
		// 3rd person turret rotation
		FVector PlayerViewPointLocation;
		FRotator PlayerViewPointRotation;
		Controller->GetPlayerViewPoint(PlayerViewPointLocation, PlayerViewPointRotation);

		// Calculate the target direction
		FVector TurretToLookDir = PlayerViewPointRotation.Vector();
		TurretToLookDir.Z = 0.f;
		if (!TurretToLookDir.IsNearlyZero())
			TurretToLookDir.Normalize();

		FVector TurretForwardVector = GetMesh()->GetSocketQuaternion("turret_jntSocket").GetForwardVector();
		TurretForwardVector.Z = 0.f;
		if (!TurretForwardVector.IsNearlyZero())
			TurretForwardVector.Normalize();

		// Calculate the target angle
		double DotProduct = FVector::DotProduct(TurretForwardVector, TurretToLookDir);

		// DO NOT CHANGE TOLERANCE
		constexpr double Tolerance = 0.01; // setting it to 0.01 fixed it now somehow when it wasnt working before. DO NOT CHANGE
		if (FMath::IsNearlyEqual(DotProduct, 1.0f, Tolerance))
			DotProduct = 1.f; // Prevent any small rounding errors
		else if (FMath::IsNearlyEqual(DotProduct, -1.0f, Tolerance))
			DotProduct = -1.f; // Handle opposite direction

		double Det = FVector::CrossProduct(TurretForwardVector, TurretToLookDir).Z;
		if (FMath::IsNearlyZero(Det, Tolerance))
			Det = 0.f;

		double TargetAngle = FMath::RadiansToDegrees(FMath::Atan2(Det, DotProduct));
		if (FMath::IsNearlyEqual(TargetAngle, 1.0, Tolerance))
			TargetAngle = 1;

		// Calculate the *difference* in angle, but now wrap it to the shortest path
		double DeltaAngle = UKismetMathLibrary::NormalizeAxis(TargetAngle - TurretAngle_);

		// Clamp the angle difference based on MaxTurretRotationSpeed
		const double MaxDeltaAngle = MaxTurretRotationSpeed_ * GetWorld()->GetDeltaSeconds();
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);

		// Update the turret angle
		MC_SetTurretRotation(TurretAngle_ + DeltaAngle);
	}
}

void ATankCharacter::TurretTurningTick(float DeltaTime)
{
	if (!Controller || !GetMesh() || !AnimInstance)
		return;

	if (!IsLocallyControlled())
		SR_TurretTurningTick(bAimingIn, AnimInstance->TurretAngle, LookValues, MaxTurretRotationSpeed);
}

void ATankCharacter::CheckIfGunCanLowerElevationTick(float DeltaTime)
{
	if (!PlayerController)
		return;
	
	FVector TopTraceStart = GetMesh()->GetSocketLocation("BarrelTraceStart");
	FVector TopTraceEnd = GetMesh()->GetSocketLocation("BarrelTraceEnd");
	FHitResult TopHit;

	// Perform a trace along the bottom of the barrel to check if it's colliding with anything
	const bool bTopHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TopTraceStart,
		TopTraceEnd,
		TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		TopHit,
		false
	);

	FVector BottomTraceStart = GetMesh()->GetSocketLocation("BarrelTrace2Start");
	FVector BottomTraceEnd = GetMesh()->GetSocketLocation("BarrelTrace2End");
	FHitResult BottomHit;

	// Perform a trace along the bottom of the barrel to check if it's colliding with anything
	const bool bBottomHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		BottomTraceStart,
		BottomTraceEnd,
		TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		BottomHit,
		false
	);

	// disable the players ability to shoot while the turret is adjusting
	if (bBottomHit == true && bTopHit == false || bBottomHit == true && bTopHit == true)
		PlayerController->SetShootingBlocked(false);

	if (bTopHit || bBottomHit) 
	{
		
		if (!TopHit.PhysMaterial.IsValid() || !BottomHit.PhysMaterial.IsValid())
			return;

		auto bTopDetectTank = TopHit.PhysMaterial->SurfaceType == SurfaceType2 && TopHit.GetActor() == this;
		auto bBottomDetectTank = BottomHit.PhysMaterial->SurfaceType == SurfaceType2 && BottomHit.GetActor() == this;

		if (bAimingIn)
		{
			// MinGunElevation = AbsoluteMinGunElevation;
			
			if (bBottomDetectTank == true && bTopDetectTank == true)
			{
				MinGunElevation = FMath::Clamp(MinGunElevation + 1, AbsoluteMinGunElevation, AbsoluteMaxGunElevation); // 17
				PlayerController->SetShootingBlocked(true);
				return;
			}

			if (bBottomDetectTank == true && bTopDetectTank == false)
			{
				MinGunElevation = GunElevation;
				PlayerController->SetShootingBlocked(false);
				return;
			}
			
		}
		else // 3rd person
		{
			if (bTopDetectTank == true) // if tank body physical material is detected and is the same actor
			{
				MinGunElevation = FMath::Clamp(MinGunElevation + 1, AbsoluteMinGunElevation, AbsoluteMaxGunElevation); // 17
				PlayerController->SetShootingBlocked(true);
			}

			if (bBottomDetectTank == true && bTopDetectTank == false)
			{
				MinGunElevation = GunElevation;
				return;
			}
		}
		
	}
	else
	{
		// Update the last free gun elevation
		// if (bAimingIn)
		// 	MinGunElevation = FMath::Max(FMath::Min(GunElevation, DesiredGunElevation), AbsoluteMinGunElevation);
		MinGunElevation = FMath::Clamp(MinGunElevation - 0.2, AbsoluteMinGunElevation, GunElevation);

		PlayerController->SetShootingBlocked(false);
	}
}

void ATankCharacter::GunElevationTick(float DeltaTime)
{
	if (!BackCameraComp)
		return;

	if (bAimingIn)
	{
		GunElevation += LookValues.Y * -1;
		GunElevation = FMath::Clamp(GunElevation, MinGunElevation, MaxGunElevation);
		SetGunElevation(GunElevation);
		return;
	}

	FVector GunLocation = BackCameraComp->GetComponentLocation() + (BackCameraComp->GetForwardVector() * 7000.0);

	FHitResult OutHit;
	auto bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		BackCameraComp->GetComponentLocation(),
		GunLocation,
		{ObjectTypeQuery1, ObjectTypeQuery6}, // should be worldstatic and destructible
		false,
		{this},
		EDrawDebugTrace::None,
		OutHit,
		true
	);

	auto LookAtRot = UKismetMathLibrary::FindLookAtRotation(
		GetMesh()->GetSocketLocation("gun_jnt"),
		bHit ? OutHit.Location : GunLocation
	);

	DesiredGunElevation = LookAtRot.Pitch;
	
	// Interpolate and clamp to the maximum allowed change rate
	GunElevation = FMath::Clamp(
		FMath::FInterpTo(GunElevation, DesiredGunElevation, DeltaTime, GunElevationInterpSpeed),
		MinGunElevation,
		MaxGunElevation
	);

	// Clamp the final value within valid elevation bounds
	GunElevation = FMath::Clamp(GunElevation, MinGunElevation, MaxGunElevation);

	// Apply the final gun elevation
	SetGunElevation(GunElevation);
}

void ATankCharacter::IsInAirTick()
{
	FVector ActorOrigin = GetActorLocation();

	FHitResult Hit;
	bIsInAir = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		ActorOrigin + FVector(0, 0, 150),
		ActorOrigin - FVector(0, 0, 40),
		TraceTypeQuery1,
		false, {this},
		EDrawDebugTrace::None,
		Hit,
		true,
		FLinearColor::Red
	);
}

void ATankCharacter::OutlineTank_Implementation(const bool bActivate)
{
	if (!GetMesh())
		return;
	
	if (bActivate)
		GetMesh()->SetCustomDepthStencilValue(1);
	else
		GetMesh()->SetCustomDepthStencilValue(0);
}

void ATankCharacter::HighlightEnemyTanksIfDetected()
{
	CurrentHitResults.Empty();
	
	FVector Start = GetMesh()->GetSocketTransform("GunShootSocket").GetLocation();
	Start.Z += LineTraceOffset;
	FVector End = Start + GetMesh()->GetSocketQuaternion("GunShootSocket").GetForwardVector() * LineTraceForwardVectorMultiplier;

	// horizontal box trace
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		HorizontalLineTraceHalfSize,
		GetMesh()->GetSocketRotation("GunShootSocket"),
		{ObjectTypeQuery5},
		false,
		{this},
		EDrawDebugTrace::None,
		HorizontalHits,
		true,
		FLinearColor::Yellow
	);

	// vertical box trace
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		VerticalLineTraceHalfSize,
		GetMesh()->GetSocketRotation("GunShootSocket"),
		{ObjectTypeQuery5},
		false,
		{this},
		EDrawDebugTrace::None,
		VerticalHits,
		true,
		FLinearColor::Yellow
	);

    // remove duplicates
    for (auto& Hit : VerticalHits)
        if (!CurrentHitResults.Contains(Hit))
            CurrentHitResults.Add(Hit);
    for (auto& Hit : HorizontalHits)
        if (!CurrentHitResults.Contains(Hit))
            CurrentHitResults.Add(Hit);

    // Removes hit results with actors that are no longer detected by the trace.
    for (auto It = HighlightedEnemyTanks.CreateIterator(); It; ++It)
    {
        if (!CurrentHitResults.Contains(*It))
        {
            // Actor no longer detected, remove it and remove outline
            Execute_OutlineTank(It->GetActor(), false);
            It.RemoveCurrent();
        }
    }

    // add any hit results that were previously not present
    for (const FHitResult& Element : CurrentHitResults)
        if (!HighlightedEnemyTanks.Contains(Element))
            HighlightedEnemyTanks.Add(Element);

    // highlight any and all actors that implement the interface
    for (const FHitResult& Hit : HighlightedEnemyTanks)
    {
        if (!Hit.IsValidBlockingHit())
            continue;

        Execute_OutlineTank(Hit.GetActor(), true); // need to change this when trying to get replication working.
    }
}

void ATankCharacter::UpdateCameraPitchLimitsTick() const
{
	if (!PlayerController)
		return;

	float TankPitch = GetActorRotation().Pitch;

	// Adjust the pitch limits based on the tank's current pitch
	float AdjustedPitchMin = BasePitchMin + TankPitch;
	float AdjustedPitchMax = BasePitchMax + TankPitch;

	// Apply the adjusted limits
	PlayerController->PlayerCameraManager->ViewPitchMin = FMath::Clamp(AdjustedPitchMin, -90.0f, 90.0f);
	PlayerController->PlayerCameraManager->ViewPitchMax = FMath::Clamp(AdjustedPitchMax, -90.0f, 90.0f);
}

void ATankCharacter::SetGunElevation(const double NewGunElevation) const
{
	if (AnimInstance == nullptr)
		return;
	
	if (HasAuthority())
		AnimInstance->GunElevation = NewGunElevation;
	else
		// Clients should not update the GunElevation directly.
		SR_SetGunElevation(NewGunElevation);
}

void ATankCharacter::SR_SetGunElevation_Implementation(double NewGunElevation) const
{
	if (AnimInstance == nullptr)
		return;

	AnimInstance->GunElevation = NewGunElevation;

	MC_SetGunElevation(NewGunElevation);
}

void ATankCharacter::MC_SetGunElevation_Implementation(double NewGunElevation) const
{
	if (AnimInstance)
		AnimInstance->GunElevation = NewGunElevation;
}

void ATankCharacter::SetTurretRotation(const double NewTurretAngle) const
{
	if (AnimInstance == nullptr)
		return;
	
	if (HasAuthority())
		AnimInstance->TurretAngle = NewTurretAngle;
	else
		SR_SetTurretRotation(NewTurretAngle);
}

void ATankCharacter::SR_SetTurretRotation_Implementation(double NewTurretAngle) const
{
	if (AnimInstance == nullptr)
		return;

	AnimInstance->TurretAngle = NewTurretAngle;
	MC_SetTurretRotation(NewTurretAngle);
}

void ATankCharacter::MC_SetTurretRotation_Implementation(double NewTurretAngle) const
{
	if (AnimInstance == nullptr)
		return;
	
	AnimInstance->TurretAngle = NewTurretAngle;
}

void ATankCharacter::SetSkinType(const double NewSkinType) const
{
	if (BodyMaterial)
		BodyMaterial->SetScalarParameterValue("SkinType", NewSkinType);
}

void ATankCharacter::SetLightsEmissivity(double LightsEmissivity) const
{
	if (BodyMaterial == nullptr)
		return;

	if (HasAuthority())
		BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
	else
		MC_SetLightsEmissivity(LightsEmissivity);
}

void ATankCharacter::MC_SetLightsEmissivity_Implementation(double LightsEmissivity) const
{
	if (BodyMaterial == nullptr)
		return;
	
	BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
}

void ATankCharacter::SetSpeed(double Speed)
{
	if (AnimInstance == nullptr)
		return;
	
	if (HasAuthority())
	{
		AnimInstance->WheelSpeed = Speed;
		SetWheelSmoke(!bIsInAir ? Speed : 0);
	}
	else
	{
		SR_SetSpeed(Speed);
	}
}

void ATankCharacter::SR_SetSpeed_Implementation(double Speed)
{
	MC_SetSpeed(Speed);
}

void ATankCharacter::MC_SetSpeed_Implementation(double Speed)
{
	if (AnimInstance == nullptr)
		return;
	
	AnimInstance->WheelSpeed = Speed;
	SetWheelSmoke(!bIsInAir ? Speed : 0);
}

void ATankCharacter::SetHatchesAngles(double HatchAngle)
{
	if (AnimInstance)
		AnimInstance->HatchAngle = HatchAngle;
}

void ATankCharacter::MC_SetWheelSmoke_Implementation(float Intensity)
{
	SetWheelSmoke(Intensity);
}

void ATankCharacter::MC_SetHatchesAngles_Implementation(double HatchAngle)
{
	SetHatchesAngles(HatchAngle);
}
