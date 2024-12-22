// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TanksCharacter.h"

#include "EnhancedInputComponent.h"
#include "TankController.h"
#include "Camera/CameraComponent.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"



// Sets default values
ATanksCharacter::ATanksCharacter(): MaxZoomIn(500), MaxZoomOut(2500)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

ATanksCharacter::~ATanksCharacter()
{
	
}

void ATanksCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ShootSocket = GetShootSocke();

	FrontCameraComp = GetFrontCamera();
	FrontSpringArmComp = GetFrontSpringArm();

	BackCameraComp = GetBackCamera();
	BackSpringArmComp = GetBackSpringArm();

	
}



void ATanksCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// cache the EnhancedInputComponent to bind controls later in BeginPlay after the controller is available.
	if (Cast<UEnhancedInputComponent>(PlayerInputComponent))
		EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

// Called when the game starts or when spawned
void ATanksCharacter::BeginPlay()
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

// Called every frame
void ATanksCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveValues = PlayerController->GetMoveValues();
	LookValues = PlayerController->GetLookValues();
}

void ATanksCharacter::SetGunElevation(double GunElevation)
{
	AnimInstance->GunElevation = GunElevation;
}

void ATanksCharacter::SetTurretRotation(double TurretAngle)
{
	AnimInstance->TurretAngle = TurretAngle;
}

void ATanksCharacter::SetSkinType(double SkinType)
{
	BodyMaterial->SetScalarParameterValue("SkinType", SkinType);
}

void ATanksCharacter::SetLightsEmissivity(double LightsEmissivity)
{
	BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
}

void ATanksCharacter::SetSpeed(double Speed)
{
	AnimInstance->WheelSpeed = Speed;

	// SetWheelSmoke(Speed);
}

void ATanksCharacter::SetHatchesAngles(double HatchAngle)
{
	AnimInstance->HatchAngle = HatchAngle;
}
