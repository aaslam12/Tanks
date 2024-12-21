// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TanksCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosVehicleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/SpringArmComponent.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"

const FName GunShootSocket = FName("gun_1_jnt");
const FName FirstPersonSocket = FName("FirstPersonSocket");

// Sets default values
ATanksCharacter::ATanksCharacter(): MaxZoomIn(500), MaxZoomOut(2500), StopTurn(false), VehicleYaw(0), bCanShoot(true)
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

	ShootSocket = GetShootSocket();
	FrontCameraComp = GetFrontCamera();
	BackCameraComp = GetBackCamera();
	SpringArmComp = GetSpringArm();

	
}

void ATanksCharacter::BindControls()
{
	// Add Input Mapping Context
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::SetupPlayerInputComponent) PlayerController is INVALID")),
		                                  true, true, FLinearColor::Red, 50);
	}

	// Set up action bindings
	if (EnhancedInputComponent)
	{

		// Moving forward and backward
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATanksCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATanksCharacter::Move);

		// Turning left and right
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ATanksCharacter::Turn);
		
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Started, this, &ATanksCharacter::TurnStarted);
		
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Canceled, this, &ATanksCharacter::TurnCompleted);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Completed, this, &ATanksCharacter::TurnCompleted);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATanksCharacter::Look);

		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ATanksCharacter::Shoot);

		// Zoom in & out
		EnhancedInputComponent->BindAction(MouseWheelUpAction, ETriggerEvent::Started, this, &ATanksCharacter::Shoot);
		EnhancedInputComponent->BindAction(MouseWheelDownAction, ETriggerEvent::Started, this, &ATanksCharacter::Shoot);

		// Handbrake
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ATanksCharacter::HandbrakeStarted);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ATanksCharacter::HandbrakeEnded);

		// Mouse Wheel Up & Down
		EnhancedInputComponent->BindAction(MouseWheelUpAction, ETriggerEvent::Started, this, &ATanksCharacter::MouseWheelUp);
		EnhancedInputComponent->BindAction(MouseWheelDownAction, ETriggerEvent::Completed, this, &ATanksCharacter::MouseWheelDown);
	}
	else
	{
		UE_LOG(LogBlueprintUserMessages, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
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

	PlayerController = Cast<APlayerController>(GetController());
	PlayerController->Possess(this);

	// stops the player from looking under the tank and above too much.
	PlayerController->PlayerCameraManager->ViewPitchMin = -60.0;
	PlayerController->PlayerCameraManager->ViewPitchMax = 15.0;

	FrontCameraComp->SetActive(false);
	BackCameraComp->SetActive(true);

	

	BindControls();
}

// Called every frame
void ATanksCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StopTurn = GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;

	SetSpeed(GetVehicleMovementComponent()->GetForwardSpeed());

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Move) MovementValues: %s"),
			*MoveValues.ToString()), true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Move) LookValues: %s"),
			*LookValues.ToString()), true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Move) StopTurn: %d"),
			StopTurn), true, true, FLinearColor::Yellow, 0);
}

void ATanksCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	MoveValues.Y = Value.GetMagnitude();

	if (MoveValues.Y >= 0)
	{
		GetVehicleMovementComponent()->SetThrottleInput(MoveValues.Y);
		GetVehicleMovementComponent()->SetBrakeInput(0);
	}
	else
	{
		GetVehicleMovementComponent()->SetThrottleInput(0);
		GetVehicleMovementComponent()->SetBrakeInput(FMath::Abs(MoveValues.Y));
	}
}


void ATanksCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		LookValues = LookAxisVector;
	}
}

void ATanksCharacter::Turn(const FInputActionValue& Value)
{
	MoveValues.X = Value.GetMagnitude();

	if (StopTurn)
		GetVehicleMovementComponent()->SetYawInput(0);
	else
	{
		VehicleYaw = MoveValues.X;
		GetVehicleMovementComponent()->SetYawInput(VehicleYaw);
	}
}

void ATanksCharacter::TurnStarted(const FInputActionValue& InputActionValue)
{
	GetVehicleMovementComponent()->SetThrottleInput(0.1);
}

void ATanksCharacter::TurnCompleted(const FInputActionValue& InputActionValue)
{
	GetVehicleMovementComponent()->SetThrottleInput(0);
	GetVehicleMovementComponent()->SetBrakeInput(0);
}

void ATanksCharacter::Shoot(const FInputActionValue& InputActionValue)
{
	if (!ShootSocket)
		return;

	if (!bCanShoot)
		return;

	for (auto ParticleSystem : ShootEmitterSystems)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, ShootSocket->GetComponentTransform());
	}
}

void ATanksCharacter::HandbrakeStarted(const FInputActionValue& InputActionValue)
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::HandbrakeStarted)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATanksCharacter::HandbrakeEnded(const FInputActionValue& InputActionValue)
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::HandbrakeEnded)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATanksCharacter::MouseWheelUp(const FInputActionValue& InputActionValue)
{
	SpringArmComp->TargetArmLength = FMath::Max(SpringArmComp->TargetArmLength - 200.0, MaxZoomIn);

	if (SpringArmComp->TargetArmLength == MaxZoomIn)
	{
		// Switch to aiming camera
		if (PlayerController && FrontCameraComp)
		{
			BackCameraComp->SetActive(false);
			FrontCameraComp->SetActive(true);
			
			// Switch to the new camera smoothly (can adjust Blend Time and Blend function)
			PlayerController->SetViewTargetWithBlend(this, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic, 0.0f);
		}
	}
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::MouseWheelUp)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATanksCharacter::MouseWheelDown(const FInputActionValue& InputActionValue)
{
	SpringArmComp->TargetArmLength = FMath::Min(SpringArmComp->TargetArmLength + 200.0, MaxZoomOut);

	if (SpringArmComp->TargetArmLength > MaxZoomIn)
	{
		// Switch to 3rd person camera
		if (PlayerController && FrontCameraComp)
		{
			FrontCameraComp->SetActive(false);
			BackCameraComp->SetActive(true);
			
			// Switch to the new camera smoothly (can adjust Blend Time and Blend function)
			PlayerController->SetViewTargetWithBlend(this, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic, 0.0f);
		}
	}

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::MouseWheelDown)")),
		true, true, FLinearColor::Yellow, 0);
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

	SetWheelSmoke(Speed);
}

void ATanksCharacter::SetHatchesAngles(double HatchAngle)
{
	AnimInstance->HatchAngle = HatchAngle;
}
