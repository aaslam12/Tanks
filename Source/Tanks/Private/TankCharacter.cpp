// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "EnhancedInputComponent.h"
#include "TankController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"



// Sets default values
ATankCharacter::ATankCharacter(): MaxZoomIn(500), MaxZoomOut(2500), MaxTurretRotationSpeed(90)
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
	PlayerController->PlayerCameraManager->ViewPitchMin = -60.0;
	PlayerController->PlayerCameraManager->ViewPitchMax = 15.0;

	FrontCameraComp->SetActive(false);
	BackCameraComp->SetActive(true);
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
		// TurretToLookDir.Normalize();
		if (!TurretToLookDir.IsNearlyZero())
			TurretToLookDir.Normalize();

		FVector TurretForwardVector = GetMesh()->GetSocketQuaternion("turret_jntSocket").GetForwardVector();
		TurretForwardVector.Z = 0.f;
		// TurretForwardVector.Normalize();
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

		// Calculate the *difference* in angle
		double DeltaAngle = TargetAngle - AnimInstance->TurretAngle;

		// Clamp the angle difference based on MaxTurretRotationSpeed
		double MaxDeltaAngle = MaxTurretRotationSpeed * DeltaTime;
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);

		AnimInstance->TurretAngle += DeltaAngle;
	}

}

void ATankCharacter::GunElevationTick(float DeltaTime)
{
	
}

// Called every frame
void ATankCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveValues = PlayerController->GetMoveValues();
	LookValues = PlayerController->GetLookValues();

	TurretTurningTick(DeltaTime);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Tick) bAimingIn: %d"), bAimingIn),
		true, true, FLinearColor::Yellow, 0);
}

void ATankCharacter::SetGunElevation(double GunElevation)
{
	AnimInstance->GunElevation = GunElevation;
}

void ATankCharacter::SetTurretRotation(double TurretAngle)
{
	AnimInstance->TurretAngle = TurretAngle;
}

void ATankCharacter::SetSkinType(double SkinType)
{
	BodyMaterial->SetScalarParameterValue("SkinType", SkinType);
}

void ATankCharacter::SetLightsEmissivity(double LightsEmissivity)
{
	BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
}

void ATankCharacter::SetSpeed(double Speed)
{
	AnimInstance->WheelSpeed = Speed;

	// SetWheelSmoke(Speed);
}

void ATankCharacter::SetHatchesAngles(double HatchAngle)
{
	AnimInstance->HatchAngle = HatchAngle;
}
