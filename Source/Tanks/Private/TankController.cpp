// Fill out your copyright notice in the Description page of Project Settings.


#include "TankController.h"

#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TankCameraManager.h"
#include "TankCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/TankHealthComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"

const FName FirstPersonSocket = FName("FirstPersonSocket");

ATankController::ATankController(): PrevTurnInput(0), LookValues(), MoveValues(), bIsInAir(true), ShootTimerDuration(3),
                                    MouseSensitivity(FVector(0.4)),
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

void ATankController::ClampVehicleSpeed()
{
	// 1. Define your max speed (in cm/s)
	//    e.g. 1000 cm/s ≈ 36 km/h
	constexpr float MaxSpeedCmPerSec = 1000.0f;  

	// 2. Get the movement component and root physics body
	if (!ChaosWheeledVehicleMovementComponent) return;

	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetPawn()->GetRootComponent());
	if (!Root || !Root->IsSimulatingPhysics()) return;

	// 3. Read current forward speed (in cm/s)
	float ForwardSpeed = ChaosWheeledVehicleMovementComponent->GetForwardSpeed();  

	// 4. If speed exceeds maximum, clamp the world-space velocity
	if (FMath::Abs(ForwardSpeed) > MaxSpeedCmPerSec)
	{
		// Compute forward axis in world space
		FVector ForwardDir = GetPawn()->GetActorForwardVector().GetSafeNormal();

		// Preserve sign (forward vs. reverse)
		float ClampedSpeed = FMath::Sign(ForwardSpeed) * MaxSpeedCmPerSec;

		// Build new velocity vector
		FVector NewVel = ForwardDir * ClampedSpeed;

		// Optionally preserve vertical velocity:
		FVector CurrentVel = Root->GetPhysicsLinearVelocity();
		NewVel.Z = CurrentVel.Z;

		// Apply clamp
		Root->SetPhysicsLinearVelocity(NewVel, false);
	}
}

void ATankController::HandleVehicleDeceleration() const
{
	// Check if movement input is zero
	if (ChaosWheeledVehicleMovementComponent && TankPlayer
		&& MoveValues.Y == 0 && MoveValues.X == 0)
	{
		const float DecelerationRate = 2500.0f; // Adjust to control the rate of deceleration
		const float CurrentForwardSpeed = ChaosWheeledVehicleMovementComponent->GetForwardSpeed();
		const float CurrentTurnInput = MoveValues.X;

		if (FMath::Abs(CurrentForwardSpeed) > KINDA_SMALL_NUMBER) // Decelerate linear motion
		{
			float DecelerationTorque = -FMath::Sign(CurrentForwardSpeed) * DecelerationRate;

			for (int32 Idx : TankPlayer->LeftWheelIndices)
			{
				ChaosWheeledVehicleMovementComponent->SetDriveTorque(DecelerationTorque, Idx);
			}
			for (int32 Idx : TankPlayer->RightWheelIndices)
			{
				ChaosWheeledVehicleMovementComponent->SetDriveTorque(DecelerationTorque, Idx);
			}
			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandleVehicleDeceleration) DecelerationTorque: %f"), DecelerationTorque),
			                                  true, true, FLinearColor::Yellow, 0);
		}

		// Smoothly reduce turning torque as well
		if (FMath::Abs(CurrentTurnInput) > KINDA_SMALL_NUMBER)
		{
			const float TurnDecelerationRate = 500.0f; // Turning deceleration rate
			float ReducedTurnTorque = -FMath::Sign(CurrentTurnInput) * TurnDecelerationRate;

			for (int32 Idx : TankPlayer->LeftWheelIndices)
			{
				ChaosWheeledVehicleMovementComponent->SetDriveTorque(ReducedTurnTorque, Idx);
			}
			for (int32 Idx : TankPlayer->RightWheelIndices)
			{
				ChaosWheeledVehicleMovementComponent->SetDriveTorque(-ReducedTurnTorque, Idx);
			}

			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandleVehicleDeceleration) ReducedTurnTorque: %f"), ReducedTurnTorque),
			                                  true, true, FLinearColor::Yellow, 0);
		}
	}

	if (ChaosWheeledVehicleMovementComponent)
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandleVehicleDeceleration) ForwardSpeed: %f"), ChaosWheeledVehicleMovementComponent->GetForwardSpeed()),
												  true, true, FLinearColor::Red, 0);
}

void ATankController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (TankPlayer && bIsAlive && VehicleMovementComponent)
    {
        bStopTurn = TankPlayer->GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;
        TankPlayer->SetSpeed(VehicleMovementComponent->GetForwardSpeed());
    }

    if (TankPlayer && TankPlayer->GetHealthComponent())
    {
        // if not dead, set bIsAlive as true
        bIsAlive = !TankPlayer->GetHealthComponent()->IsDead();
    }

    ClampVehicleSpeed();
    HandleVehicleDeceleration();
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
	{
		VehicleMovementComponent = TankPlayer->GetVehicleMovementComponent();
		ChaosWheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovementComponent);
	}
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

	const float MaxTorquePerWheel = ChaosWheeledVehicleMovementComponent->EngineSetup.MaxTorque;

	// Spin-in-place formula:
	float LeftPower  = Value;
	float RightPower = Value;
		
	for (int32 Idx : TankPlayer->LeftWheelIndices)
		ChaosWheeledVehicleMovementComponent->SetDriveTorque(LeftPower * MaxTorquePerWheel, Idx);
	
	for (int32 Idx : TankPlayer->RightWheelIndices)
		ChaosWheeledVehicleMovementComponent->SetDriveTorque(RightPower * MaxTorquePerWheel, Idx);

	ClampVehicleSpeed();
	HandleVehicleDeceleration();
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
	MoveValues.X = Value.GetMagnitude();
	bIsInAir = !TankPlayer->IsInAir();
	
	VehicleMovementComponent->SetYawInput(MoveValues.X);

	ClampVehicleSpeed();
	HandleVehicleDeceleration();
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
	MoveValues.X = 0;
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