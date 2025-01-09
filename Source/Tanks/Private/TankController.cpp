// Fill out your copyright notice in the Description page of Project Settings.


#include "TankController.h"

#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TankCameraManager.h"
#include "TankCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

const FName FirstPersonSocket = FName("FirstPersonSocket");

ATankController::ATankController(): bIsAlive(true), LookValues(), MoveValues(), bCanMove(true), ShootTimerDuration(3),
                                    MouseSensitivity(FVector(0.4)),
                                    bStopTurn(false),
                                    VehicleYaw(0),
                                    bCanShoot(true), bShootingBlocked(false)
{
	if (TankCameraManagerClass)
		PlayerCameraManagerClass = TankCameraManagerClass;

	bReplicates = true;
}

void ATankController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ATankController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TankPlayer && bIsAlive)
	{
		bStopTurn = TankPlayer->GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;
		TankPlayer->SetSpeed(TankPlayer->GetVehicleMovementComponent()->GetForwardSpeed());
	}
}

void ATankController::SetDefaults()
{
	TankPlayer = Cast<ATankCharacter>(GetPawn());
	
	if (!TankPlayer)
		return;
	
	if (TankPlayer->GetBackSpringArmComp()->TargetArmLength == TankPlayer->GetMaxZoomIn())
	{
		TankPlayer->bAimingIn = true;
	}
	else if (TankPlayer->GetBackSpringArmComp()->TargetArmLength > TankPlayer->GetMaxZoomIn())
	{
		TankPlayer->bAimingIn = false;
	}
}

void ATankController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetDefaults();
	BindControls();
}

void ATankController::BeginPlay()
{
	Super::BeginPlay();
	SetDefaults();
	BindControls();
}

void ATankController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		Subsystem->AddMappingContext(DefaultMappingContext, 0);

	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(EnhancedInputComponent, TEXT("(ATankController::SetupInputComponent) EnhancedInputComponent is NULL!!!"));
}

void ATankController::OnDie_Implementation()
{
	SetCanShoot(false);
	bIsAlive = true;
	ShootTimerHandle.Invalidate();
}

void ATankController::OnRespawn_Implementation()
{
}

void ATankController::BindControls()
{
	// Set up action bindings
	if (EnhancedInputComponent && TankPlayer)
	{
		// Moving forward and backward
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATankController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Ongoing, this, &ATankController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATankController::Move);

		// Turning left and right
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ATankController::Turn);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Completed, this, &ATankController::Turn);
		
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Started, this, &ATankController::TurnStarted);
		
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Canceled, this, &ATankController::TurnCompleted);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Completed, this, &ATankController::TurnCompleted);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATankController::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ATankController::Look);

		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ATankController::Shoot);

		// Zoom in & out
		EnhancedInputComponent->BindAction(MouseWheelUpAction, ETriggerEvent::Started, this, &ATankController::MouseWheelUp);
		EnhancedInputComponent->BindAction(MouseWheelDownAction, ETriggerEvent::Completed, this, &ATankController::MouseWheelDown);

		// Handbrake
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ATankController::HandbrakeStarted);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ATankController::HandbrakeEnded);
	}
}

void ATankController::Move(const FInputActionValue& Value)
{
	if (!TankPlayer || !bIsAlive)
		return;

	MoveValues.Y = Value.GetMagnitude();
	bCanMove = !TankPlayer->IsInAir();
	
	// if (bCanMove == false)
	// 	return;

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
	

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::Move) Move: %s"), *MoveValues.ToString()),
	// 		true, true, FLinearColor::Yellow, 0);
}

void ATankController::Look(const FInputActionValue& Value)
{
	if (!TankPlayer || !bIsAlive)
		return;
	
	// input is a Vector2D
	LookValues = Value.Get<FVector2D>() * MouseSensitivity;

	// add yaw and pitch input to controller
	AddYawInput(LookValues.X);
	AddPitchInput(LookValues.Y);
}

void ATankController::Turn(const FInputActionValue& Value)
{
	if (!TankPlayer || !bIsAlive)
		return;
    		
	MoveValues.X = Value.GetMagnitude();
	bCanMove = !TankPlayer->IsInAir();
	
	// if (bCanMove == false)
	// 	return;

	if (bStopTurn)
		TankPlayer->GetVehicleMovementComponent()->SetYawInput(0);
	else
	{
		VehicleYaw = MoveValues.X;
		TankPlayer->GetVehicleMovementComponent()->SetYawInput(VehicleYaw * 2);
	}
}

void ATankController::TurnStarted(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer || !bIsAlive)
		return;
	
	TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0.1);
}

void ATankController::TurnCompleted(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer || !bIsAlive)
		return;
	
	TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0);
	TankPlayer->GetVehicleMovementComponent()->SetBrakeInput(0);
}

void ATankController::StartShootTimer()
{
	FTimerDelegate ShootTimerDelegate = FTimerDelegate();
	ShootTimerDelegate.BindUFunction(this, "SetCanShoot", true);

	// set a timer to reenable shooting
	GetWorld()->GetTimerManager().SetTimer(
		ShootTimerHandle, ShootTimerDelegate,
		ShootTimerDuration, false
	);
}

void ATankController::Shoot(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer || !bIsAlive)
		return;
	
	if (!TankPlayer->GetShootSocket())
		return;

	if (bCanShoot == false || bShootingBlocked == true)
		return;
	
	// disable shooting
	SetCanShoot(false);
	StartShootTimer();

	OnShoot.Broadcast();
}

void ATankController::HandbrakeStarted(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer || !bIsAlive)
		return;
	
	TankPlayer->GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void ATankController::HandbrakeEnded(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer || !bIsAlive)
		return;
    		
	TankPlayer->GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void ATankController::MouseWheelUp(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;

	if (!TankPlayer->GetBackSpringArmComp() || !TankPlayer->GetFrontCameraComp())
		return;

	auto BackSpringArmComp = TankPlayer->GetBackSpringArmComp();

	if (BackSpringArmComp->TargetArmLength == TankPlayer->GetMaxZoomIn())
		return;
	
	BackSpringArmComp->TargetArmLength = FMath::Max(BackSpringArmComp->TargetArmLength - 200.0, TankPlayer->GetMaxZoomIn());

	if (BackSpringArmComp->TargetArmLength == TankPlayer->GetMaxZoomIn())
	{
		// Switch to aiming camera
		if (TankPlayer->GetFrontCameraComp() && bIsAlive)
		{
			TankPlayer->GetFrontCameraComp()->SetActive(true);
			TankPlayer->GetBackCameraComp()->SetActive(false);
			
			// Switch to the new camera smoothly (can adjust Blend Time and Blend function)
			SetViewTarget(TankPlayer);

			TankPlayer->bAimingIn = true;
			TankPlayer->SetMinGunElevation(TankPlayer->GetAbsoluteMinGunElevation()); // reset the value. will be overwritten in tick later
		}
	}
}

void ATankController::MouseWheelDown(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;

	if (!TankPlayer->GetBackSpringArmComp() || !TankPlayer->GetFrontCameraComp())
		return;

	auto BackSpringArmComp = TankPlayer->GetBackSpringArmComp();

	if (BackSpringArmComp->TargetArmLength == TankPlayer->GetMaxZoomOut())
		return;
	
	BackSpringArmComp->TargetArmLength = FMath::Min(BackSpringArmComp->TargetArmLength + 200.0, TankPlayer->GetMaxZoomOut());

	if (BackSpringArmComp->TargetArmLength > TankPlayer->GetMaxZoomIn())
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
}

FVector2D ATankController::GetMoveValues() const
{
	return MoveValues;
}

FVector2D ATankController::GetLookValues() const
{
	return LookValues;
}
