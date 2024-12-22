// Fill out your copyright notice in the Description page of Project Settings.


#include "TankController.h"

#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TankCameraManager.h"
#include "TankCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

const FName GunShootSocket = FName("gun_1_jnt");
const FName FirstPersonSocket = FName("FirstPersonSocket");

ATankController::ATankController(): bStopTurn(false), VehicleYaw(0), bCanShoot(true)
{
	if (TankCameraManagerClass)
		PlayerCameraManagerClass = TankCameraManagerClass;
}

void ATankController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TankPlayer)
	{
		bStopTurn = TankPlayer->GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;
		TankPlayer->SetSpeed(TankPlayer->GetVehicleMovementComponent()->GetForwardSpeed());
	}

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::Tick) MovementValues: %s"),
			*MoveValues.ToString()), true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::Tick) LookValues: %s"),
			*LookValues.ToString()), true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::Tick) StopTurn: %d"),
			bStopTurn), true, true, FLinearColor::Yellow, 0);
}

void ATankController::BeginPlay()
{
	Super::BeginPlay();
	TankPlayer = Cast<ATankCharacter>(GetPawn());
	checkf(TankPlayer, TEXT("TankPlayer is invalid in ATankController::BeginPlay:49"));

	BindControls();
}

void ATankController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
}

void ATankController::BindControls()
{
	// Set up action bindings
	if (EnhancedInputComponent)
	{

		// Moving forward and backward
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATankController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATankController::Move);

		// Turning left and right
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ATankController::Turn);
		
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Started, this, &ATankController::TurnStarted);
		
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Canceled, this, &ATankController::TurnCompleted);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Completed, this, &ATankController::TurnCompleted);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATankController::Look);

		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ATankController::Shoot);

		// Zoom in & out
		EnhancedInputComponent->BindAction(MouseWheelUpAction, ETriggerEvent::Started, this, &ATankController::MouseWheelUp);
		EnhancedInputComponent->BindAction(MouseWheelDownAction, ETriggerEvent::Completed, this, &ATankController::MouseWheelDown);

		// Handbrake
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ATankController::HandbrakeStarted);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ATankController::HandbrakeEnded);
		
	}
	else
	{
		UE_LOG(LogBlueprintUserMessages, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void ATankController::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	MoveValues.Y = Value.GetMagnitude();

	if (MoveValues.Y >= 0)
	{
		TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(MoveValues.Y);
		TankPlayer->GetVehicleMovementComponent()->SetBrakeInput(0);
	}
	else
	{
		TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0);
		TankPlayer->GetVehicleMovementComponent()->SetBrakeInput(FMath::Abs(MoveValues.Y));
	}
}


void ATankController::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	LookValues = Value.Get<FVector2D>();

	// add yaw and pitch input to controller
	AddYawInput(LookValues.X);
	AddPitchInput(LookValues.Y);
}

void ATankController::Turn(const FInputActionValue& Value)
{
	MoveValues.X = Value.GetMagnitude();

	if (bStopTurn)
		TankPlayer->GetVehicleMovementComponent()->SetYawInput(0);
	else
	{
		VehicleYaw = MoveValues.X;
		TankPlayer->GetVehicleMovementComponent()->SetYawInput(VehicleYaw);
	}
}

void ATankController::TurnStarted(const FInputActionValue& InputActionValue)
{
	TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0.1);
}

void ATankController::TurnCompleted(const FInputActionValue& InputActionValue)
{
	TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0);
	TankPlayer->GetVehicleMovementComponent()->SetBrakeInput(0);
}

void ATankController::Shoot(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer->GetShootSocket())
		return;

	if (!bCanShoot)
		return;

	for (auto ParticleSystem : TankPlayer->GetShootEmitterSystems())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, TankPlayer->GetShootSocket()->GetComponentTransform());

		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::Shoot) SpawnEmitterAtLocation: %s"), *ParticleSystem.GetFullName()),
		true, true, FLinearColor::Yellow, 0);
	}
}

void ATankController::HandbrakeStarted(const FInputActionValue& InputActionValue)
{
	TankPlayer->GetVehicleMovementComponent()->SetHandbrakeInput(true);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandbrakeStarted)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATankController::HandbrakeEnded(const FInputActionValue& InputActionValue)
{
	TankPlayer->GetVehicleMovementComponent()->SetHandbrakeInput(false);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandbrakeEnded)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATankController::MouseWheelUp(const FInputActionValue& InputActionValue)
{
	TankPlayer->GetBackSpringArmComp()->TargetArmLength = FMath::Max(TankPlayer->GetBackSpringArmComp()->TargetArmLength - 200.0, TankPlayer->GetMaxZoomIn());

	if (TankPlayer->GetBackSpringArmComp()->TargetArmLength == TankPlayer->GetMaxZoomIn())
	{
		// Switch to aiming camera
		if (TankPlayer->GetFrontCameraComp())
		{
			TankPlayer->GetFrontCameraComp()->SetActive(true);
			TankPlayer->GetBackCameraComp()->SetActive(false);
			
			// Switch to the new camera smoothly (can adjust Blend Time and Blend function)
			SetViewTarget(TankPlayer);

			TankPlayer->bAimingIn = true;
		}
	}
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::MouseWheelUp)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATankController::MouseWheelDown(const FInputActionValue& InputActionValue)
{
	TankPlayer->GetBackSpringArmComp()->TargetArmLength = FMath::Min(TankPlayer->GetBackSpringArmComp()->TargetArmLength + 200.0, TankPlayer->GetMaxZoomOut());

	if (TankPlayer->GetBackSpringArmComp()->TargetArmLength > TankPlayer->GetMaxZoomIn())
	{
		// Switch to 3rd person camera
		if (TankPlayer->GetBackCameraComp())
		{
			TankPlayer->GetFrontCameraComp()->SetActive(false);
			TankPlayer->GetBackCameraComp()->SetActive(true);
			
			// Switch to the new camera smoothly (can adjust Blend Time and Blend function)
			SetViewTarget(TankPlayer);

			TankPlayer->bAimingIn = false;
		}
	}

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::MouseWheelDown)")),
		true, true, FLinearColor::Yellow, 0);
}

FVector2D ATankController::GetMoveValues() const
{
	return MoveValues;
}

FVector2D ATankController::GetLookValues() const
{
	return LookValues;
}
