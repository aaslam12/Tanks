// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "EnhancedInputComponent.h"
#include "TankController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"


// Sets default values
ATankCharacter::ATankCharacter(): MaxZoomIn(500), MaxZoomOut(2500), MinGunElevation(-15), MaxGunElevation(20),
                                  CurrentMinGunElevation(-15),
                                  MaxTurretRotationSpeed(90), GunElevationInterpSpeed(10),
                                  GunElevation(0), bIsInAir(false), LastFreeGunElevation(0), DesiredGunElevation(0),
                                  LineTraceOffset(0), LineTraceForwardVectorMultiplier(8000),
                                  VerticalLineTraceHalfSize(FVector(10, 10, 300)),
                                  HorizontalLineTraceHalfSize(FVector(10, 300, 10)),
                                  bAimingIn(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
}

ATankCharacter::~ATankCharacter()
{
	
}

void ATankCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ShootSocket = GetShootSocke();

	FrontCameraComp = GetFrontCamera();
	FrontSpringArmComp = GetFrontSpringArm();

	BackCameraComp = GetBackCamera();
	BackSpringArmComp = GetBackSpringArm();

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
		PlayerController->PlayerCameraManager->ViewPitchMin = -20.0;
		PlayerController->PlayerCameraManager->ViewPitchMax = 15.0;
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

void ATankCharacter::TurretTurningTick(float DeltaTime) const
{
	if (!Controller || !GetMesh() || !AnimInstance)
		return;
	
	if (bAimingIn)
	{
		// first person turret rotation
		
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
		double DeltaAngle = UKismetMathLibrary::NormalizeAxis(TargetAngle - AnimInstance->TurretAngle);

		// Clamp the angle difference based on MaxTurretRotationSpeed
		const double MaxDeltaAngle = MaxTurretRotationSpeed * DeltaTime;
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);

		// Update the turret angle
		AnimInstance->TurretAngle += DeltaAngle;
	}

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
	if (bBottomHit == true && bTopHit == false)
		PlayerController->SetShootingBlocked(false);

	if (bTopHit || bBottomHit) 
	{
		
		if (!TopHit.PhysMaterial.IsValid() || !BottomHit.PhysMaterial.IsValid())
			return;

		auto bTopDetectTank = TopHit.PhysMaterial->SurfaceType == SurfaceType2 && TopHit.GetActor() == this;
		
		if (bTopDetectTank == true) // if tank body physical material is detected and is the same actor
		{
			CurrentMinGunElevation += 0.5; // 17
			MinGunElevation = CurrentMinGunElevation;
			PlayerController->SetShootingBlocked(true);
		}
	}
	else
	{
		// Update the last free gun elevation
		CurrentMinGunElevation = GunElevation;
		MinGunElevation = FMath::Min(GunElevation, DesiredGunElevation);
		PlayerController->SetShootingBlocked(false);
	}
}

void ATankCharacter::GunElevationTick(float DeltaTime)
{
	if (!BackCameraComp)
		return;
	
	FVector GunLocation = BackCameraComp->GetComponentLocation() + (BackCameraComp->GetForwardVector() * 7000.0);

	FHitResult OutHit;
	auto bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		BackCameraComp->GetComponentLocation(),
		GunLocation,
		{ObjectTypeQuery1, ObjectTypeQuery6}, // should be worldstatic and destructible
		false,
		{this},
		EDrawDebugTrace::ForOneFrame,
		OutHit,
		true
	);

	auto LookAtRot = UKismetMathLibrary::FindLookAtRotation(
		GetMesh()->GetSocketLocation("gun_jnt"),
		bHit ? OutHit.Location : GunLocation
	);

	DesiredGunElevation = LookAtRot.Pitch;
	
	GunElevation = FMath::Clamp(
		UKismetMathLibrary::FInterpTo(GunElevation, LookAtRot.Pitch, DeltaTime, GunElevationInterpSpeed),
		MinGunElevation,
		MaxGunElevation
	);

	SetGunElevation(GunElevation);
}

void ATankCharacter::IsInAirTick()
{
	FVector ActorOrigin = GetActorLocation();

	FHitResult Hit;
	bIsInAir = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		ActorOrigin + FVector(0, 0, 150),
		ActorOrigin - FVector(0, 0, 75),
		TraceTypeQuery1,
		false, {this},
		EDrawDebugTrace::ForOneFrame,
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
		EDrawDebugTrace::ForOneFrame,
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
		EDrawDebugTrace::ForOneFrame,
		VerticalHits,
		true,
		FLinearColor::Yellow
	);

	// removes duplicates
	for (auto Hit : VerticalHits)
		CurrentHitResults.Add(Hit);
	for (auto Hit : HorizontalHits)
		CurrentHitResults.Add(Hit);

	// removes hit results with actors that are no longer detected by the trace.
	for (auto It = SortedHitResults.CreateIterator(); It; ++It)
	{
		if (!CurrentHitResults.Contains(*It))
		{
			// Actor no longer detected, remove it and remove outline
			Execute_OutlineTank(It->GetActor(), false);
			It.RemoveCurrent();
		}
	}

	// adding any actors that are not already in the SortedHitResults
	for (const FHitResult& Element : CurrentHitResults)
		SortedHitResults.Add(Element);

	// loop through SortedHitResults to highlight any and all actors that implement the interface
	for (const FHitResult& Hit : SortedHitResults)
	{
		if (!Hit.IsValidBlockingHit())
			continue;

		Execute_OutlineTank(Hit.GetActor(), true);
	}
}

// Called every frame
void ATankCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld())
		return;

	if (PlayerController)
	{
		MoveValues = PlayerController->GetMoveValues();
		LookValues = PlayerController->GetLookValues();
	}

	TurretTurningTick(DeltaTime);
	GunElevationTick(DeltaTime);
	CheckIfGunCanLowerElevationTick(DeltaTime);
	
	FVector2D GunTraceScreenPosition;
	FVector GunTraceEndpoint;
	GunSightTick(GunTraceEndpoint, GunTraceScreenPosition);
	
	IsInAirTick();
	HighlightEnemyTanksIfDetected();

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Tick) GunTraceEndpoint: %s"), *GunTraceEndpoint.ToString()),
	// 	true, true, FLinearColor::Yellow, 0);
}

void ATankCharacter::SetGunElevation(const double NewGunElevation) const
{
	if (AnimInstance)
		AnimInstance->GunElevation = NewGunElevation;
}

void ATankCharacter::SetTurretRotation(const double NewTurretAngle) const
{
	if (AnimInstance)
		AnimInstance->TurretAngle = NewTurretAngle;
}

void ATankCharacter::SetSkinType(const double NewSkinType) const
{
	if (BodyMaterial)
		BodyMaterial->SetScalarParameterValue("SkinType", NewSkinType);
}

void ATankCharacter::SetLightsEmissivity(double LightsEmissivity) const
{
	if (BodyMaterial)
		BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
}

void ATankCharacter::SetSpeed(double Speed)
{
	if (!AnimInstance)
		return;
	
	AnimInstance->WheelSpeed = Speed;
	SetWheelSmoke(!bIsInAir ? Speed : 0);
}

void ATankCharacter::SetHatchesAngles(double HatchAngle)
{
	if (AnimInstance)
		AnimInstance->HatchAngle = HatchAngle;
}
