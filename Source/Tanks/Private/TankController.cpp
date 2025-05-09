// Fill out your copyright notice in the Description page of Project Settings.


#include "TankController.h"

#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TankCameraManager.h"
#include "TankCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/TankHealthComponent.h"
#include "GameFramework/SpringArmComponent.h"

const FName FirstPersonSocket = FName("FirstPersonSocket");

ATankController::ATankController(): LookValues(), MoveValues(), bIsInAir(true), ShootTimerDuration(3), MouseSensitivity(FVector(0.4)),
                                    bIsAlive(true),
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

	if (TankPlayer && bIsAlive && VehicleMovementComponent)
	{
		bStopTurn = TankPlayer->GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;
		TankPlayer->SetSpeed(VehicleMovementComponent->GetForwardSpeed());
	}

	if (TankPlayer)
		if (TankPlayer->GetHealthComponent())
		{
			// if not dead, set bIsAlive as true
			bIsAlive = !TankPlayer->GetHealthComponent()->IsDead();
		}
}

void ATankController::SetDefaults()
{
	bCanShoot = true;
	if (GetPawn())
		TankPlayer = Cast<ATankCharacter>(GetPawn());
	
	if (!TankPlayer)
		return;
	
	TankPlayer->bAimingIn = TankPlayer->IsAimingIn();

	if (TankPlayer->GetVehicleMovementComponent())
		VehicleMovementComponent = TankPlayer->GetVehicleMovementComponent();
}

void ATankController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// SetupInput();
	SetDefaults();
	// BindControls();
}

void ATankController::SetupInput()
{
	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		if (!Subsystem->HasMappingContext(DefaultMappingContext))
			Subsystem->AddMappingContext(DefaultMappingContext, 0);

	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(EnhancedInputComponent, TEXT("(ATankController::SetupInputComponent) EnhancedInputComponent is NULL!!!"));

	BindControls();
}

void ATankController::BeginPlay()
{
	Super::BeginPlay();
}

void ATankController::SetupInputComponent()
{
	Super::SetupInputComponent();

	SetupInput();
}

void ATankController::OnDie_Implementation()
{
	SetCanShoot(false);
	bIsAlive = false;
	ShootTimerHandle.Invalidate();
}

void ATankController::OnRespawn_Implementation()
{
	SetDefaults();
}

void ATankController::BindControls()
{
	// Set up action bindings
	if (EnhancedInputComponent)
	{
		// Moving forward and backward
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATankController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Ongoing, this, &ATankController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATankController::Move);

		// Turning left and right
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ATankController::Turn);
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

		// Self Destruct
		EnhancedInputComponent->BindAction(SelfDestructAction, ETriggerEvent::Started, this, &ATankController::SelfDestruct);
	}
}

bool ATankController::CanRegisterInput() const
{
	return TankPlayer && bIsAlive && VehicleMovementComponent;
}

void ATankController::MC_Move_Implementation(double Value)
{
	Move__Internal(Value);
}

void ATankController::Move__Internal(double Value)
{
	MoveValues.Y = Value;
	
	if (Value >= 0)
	{
		VehicleMovementComponent->SetThrottleInput(Value);
		VehicleMovementComponent->SetBrakeInput(0);
	}
	else
	{
		VehicleMovementComponent->SetThrottleInput(0);
		VehicleMovementComponent->SetBrakeInput(FMath::Abs(Value));
	}
}

void ATankController::Move(const FInputActionValue& Value)
{
	if (!CanRegisterInput())
		return;

	bIsInAir = !TankPlayer->IsInAir();
	Move__Internal(Value.GetMagnitude());
}

void ATankController::SR_Move_Implementation(double Value)
{
	Move__Internal(Value);
}

void ATankController::Look(const FInputActionValue& Value)
{
	if (!CanRegisterInput())
		return;
	
	// input is a Vector2D
	LookValues = Value.Get<FVector2D>() * MouseSensitivity;

	// add yaw and pitch input to controller
	AddYawInput(LookValues.X);
	AddPitchInput(LookValues.Y);
}

void ATankController::Turn__Internal(const FInputActionValue& Value)
{
	if (!FMath::IsNearlyEqual(PrevTurnInput, Value.GetMagnitude()))
	{
		PrevTurnInput = Value.GetMagnitude();
		
		MoveValues.X = Value.GetMagnitude();
		bIsInAir = !TankPlayer->IsInAir();
	
		VehicleMovementComponent->SetYawInput(MoveValues.X);
	}
	else
	{
		// it is same. do nothing
	}
}

void ATankController::SR_Turn_Implementation(const FInputActionValue& Value)
{
	Turn__Internal(Value);
}

void ATankController::SR_TurnCompleted_Implementation(const FInputActionValue& Value)
{
	// code should be the same as ATankController::TurnCompleted
	VehicleMovementComponent->SetThrottleInput(0);
	VehicleMovementComponent->SetBrakeInput(0);
	VehicleMovementComponent->SetYawInput(0);
	
	PrevTurnInput = 0; // fixes a bug. keep it
}

void ATankController::Turn(const FInputActionValue& Value)
{
	if (!CanRegisterInput())
		return;

	Turn__Internal(Value);
	SR_Turn(Value);
}

void ATankController::TurnCompleted(const FInputActionValue& Value)
{
	VehicleMovementComponent->SetThrottleInput(0);
	VehicleMovementComponent->SetBrakeInput(0);
	VehicleMovementComponent->SetYawInput(0);
	
	PrevTurnInput = 0; // fixes a bug. keep it

	SR_TurnCompleted(Value);
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
	if (!CanRegisterInput())
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

void ATankController::SelfDestruct(const FInputActionValue& InputActionValue)
{
	if (!CanRegisterInput())
		return;
	
	if (auto e = TankPlayer->GetHealthComponent())
		e->SelfDestruct(-1);
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
