// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "EnhancedInputComponent.h"
#include "TankController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"
#include "KismetProceduralMeshLibrary.h"



// Sets default values
ATankCharacter::ATankCharacter(): MaxZoomIn(500), MaxZoomOut(2500), MinGunElevation(-15), MaxGunElevation(20),
                                  CurrentMinGunElevation(-15),
                                  MaxTurretRotationSpeed(90), GunElevationInterpSpeed(10),
                                  GunElevation(0), bIsInAir(false), LastFreeGunElevation(0), DesiredGunElevation(0),
                                  bAimingIn(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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

	// Enable custom depth for the tank mesh
	GetMesh()->SetRenderCustomDepth(true);
	// Set the custom depth stencil value to differentiate between different types of objects
	GetMesh()->SetCustomDepthStencilValue(1);
}

void ATankCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// cache the EnhancedInputComponent to bind controls later in BeginPlay after the controller is available.
	if (Cast<UEnhancedInputComponent>(PlayerInputComponent))
		EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

// Called when the game starts or when spawned
void ATankCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetActorScale3D(FVector(0.95));
	AnimInstance = CastChecked<UTankAnimInstance>(GetMesh()->GetAnimInstance());

	PlayerController = Cast<ATankController>(GetController());
	PlayerController->Possess(this);

	// stops the player from looking under the tank and above too much.
	PlayerController->PlayerCameraManager->ViewPitchMin = -20.0;
	PlayerController->PlayerCameraManager->ViewPitchMax = 15.0;

	FrontCameraComp->SetActive(false);
	BackCameraComp->SetActive(true);

	SetLightsEmissivity(0);
}

void ATankCharacter::TurretTurningTick(float DeltaTime)
{
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

		double Tolerance = 0.05;
		if (FMath::IsNearlyEqual(DotProduct, 1.0f, 0.01f))
			DotProduct = 1.f; // Prevent any small rounding errors
		else if (FMath::IsNearlyEqual(DotProduct, -1.0f, 0.01f))
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
		double MaxDeltaAngle = MaxTurretRotationSpeed * DeltaTime;
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);

		// Update the turret angle
		AnimInstance->TurretAngle += DeltaAngle;
	}

}

void ATankCharacter::CheckIfGunCanLowerElevationTick(float DeltaTime)
{
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
	GunLocation = BackCameraComp->GetComponentLocation() + (BackCameraComp->GetForwardVector() * 7000.0);

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

void ATankCharacter::HighlightEnemyTank(AActor* EnemyTank)
{
}

void ATankCharacter::FindEnemyTanks(const FVector2D& GunTraceScreenPosition)
{
	auto Triangles = TArray<int32>{};
	auto TraceLocations = TArray<FVector>{};
	auto UVs = TArray<FVector2D>{};
	UKismetProceduralMeshLibrary::CreateGridMeshWelded(4, 4, Triangles, TraceLocations, UVs, 16.0);

	for (const auto& Location : TraceLocations)
	{
		auto temp = Location + GetActorLocation();
		temp.Z += 300;
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::FindEnemyTanks) TraceLocations: %s"), *temp.ToString()),
			true, true, FLinearColor::Yellow, 0);

		TArray<FHitResult> Hits;
		UKismetSystemLibrary::LineTraceMultiByProfile(
			GetWorld(),
			temp,
			temp + PlayerController->PlayerCameraManager->GetActorForwardVector() * 7000,
			TEXT("Vehicle"),
			false,
			{this},
			EDrawDebugTrace::ForOneFrame,
			Hits,
			true,
			FLinearColor::Blue
		);

		for (const FHitResult& Hit : Hits)
		{
			if (!Hit.IsValidBlockingHit())
				continue;

			if (IsEnemyNearTankCrosshair(Hit.Location, GunTraceScreenPosition))
				HighlightEnemyTank(Hit.GetActor());
		}
	}
}

bool ATankCharacter::IsEnemyNearTankCrosshair(const FVector& EnemyTankLocation, const FVector2D& CrosshairScreenPosition)
{
	// checkf(0, TEXT("IMPLEMENT ATankCharacter::IsEnemyNearTankCrosshairTick"));

	FVector2D EnemyScreenPosition;
	PlayerController->ProjectWorldLocationToScreen(EnemyTankLocation, EnemyScreenPosition);

	auto DistanceOnScreen = FVector2D::Distance(CrosshairScreenPosition, EnemyScreenPosition);

	if (DistanceOnScreen < 100) // if less than 100 pixels away
		HighlightEnemyTank(nullptr); // get tank reference here
	else
		HighlightEnemyTank(nullptr); // nullptr will be used to indicate no tank is to be outlined.
	
	return false;
}

// Called every frame
void ATankCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveValues = PlayerController->GetMoveValues();
	LookValues = PlayerController->GetLookValues();

	TurretTurningTick(DeltaTime);
	GunElevationTick(DeltaTime);
	CheckIfGunCanLowerElevationTick(DeltaTime);
	
	FVector2D GunTraceScreenPosition;
	FVector GunTraceEndpoint;
	GunSightTick(GunTraceEndpoint, GunTraceScreenPosition);
	
	IsInAirTick();
	FindEnemyTanks(GunTraceScreenPosition);

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Tick) GunTraceEndpoint: %s"), *GunTraceEndpoint.ToString()),
	// 	true, true, FLinearColor::Yellow, 0);
	//
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Tick) GunTraceScreenPosition: %s"), *GunTraceScreenPosition.ToString()),
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
