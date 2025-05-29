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

const FName FirstPersonSocket = FName("FirstPersonSocket");

ATankController::ATankController(): PrevTurnInput(0), LookValues(), MoveValues(), bIsInAir(true),
                                    bInputMasterSwitch(true), bDecelerateWhenIdle(true), BaseMaxSpeed(1000),
                                    CurrentMaxSpeedLimit(0),
                                    DecelerationRate(500),
                                    ShootTimerDuration(3),
                                    MouseSensitivity(0.4),
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

void ATankController::ClampVehicleSpeed() const
{
	if (!ChaosWheeledVehicleMovementComponent && !GetPawn())
		return;

	float ForwardSpeed = ChaosWheeledVehicleMovementComponent->GetForwardSpeed();  

	if (FMath::Abs(ForwardSpeed) <= BaseMaxSpeed)
		return;

	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetPawn()->GetRootComponent());
	
	if (!Root || !Root->IsSimulatingPhysics())
		return;

	FVector ForwardDir = GetPawn()->GetActorForwardVector().GetSafeNormal();

	// Preserve sign (forward or reverse)
	float ClampedSpeed = FMath::Sign(ForwardSpeed) * BaseMaxSpeed;

	FVector NewVel = ForwardDir * ClampedSpeed;

	// Preserves vertical velocity
	FVector CurrentVel = Root->GetPhysicsLinearVelocity();
	NewVel.Z = CurrentVel.Z;

	Root->SetPhysicsLinearVelocity(NewVel, false);
}

void ATankController::SetDriveTorque(const float DecelerationTorque) const
{
	SetDriveTorque(DecelerationTorque, DecelerationTorque);
}

void ATankController::SetDriveTorque(const float LeftDecelerationTorque, const float RightDecelerationTorque) const
{
	for (const int32 Idx : TankPlayer->LeftWheelIndices)
		ChaosWheeledVehicleMovementComponent->SetDriveTorque(LeftDecelerationTorque, Idx);
			
	for (const int32 Idx : TankPlayer->RightWheelIndices)
		ChaosWheeledVehicleMovementComponent->SetDriveTorque(RightDecelerationTorque, Idx);
}

double ATankController::CalculateDecelerationCurveMultiplier(const UCurveFloat* CurveFloat) const
{
	if (!ChaosWheeledVehicleMovementComponent || !CurveFloat)
		return DOUBLE_BIG_NUMBER;

	const auto AbsSpeed = FMath::Abs(ChaosWheeledVehicleMovementComponent->GetForwardSpeed());

	// Normalize speed to get a 0-1 value for the curve
	const double NormalizedSpeed = FMath::Clamp(AbsSpeed / BaseMaxSpeed, 0.0f, 1.0f);
	const double CurveMultiplier = DecelerationCurve->GetFloatValue(NormalizedSpeed);
	
	return CurveMultiplier;
}

void ATankController::HandleVehicleDeceleration()
{
	if (!CanRegisterInput())
		return;

	// Only apply deceleration when idle and the feature is enabled
	if (MoveValues.Y == 0 && MoveValues.X == 0 && bDecelerateWhenIdle)
	{
		const float CurrentForwardSpeed = ChaosWheeledVehicleMovementComponent->GetForwardSpeed();
		const float AbsSpeed = FMath::Abs(CurrentForwardSpeed);
		float ActualDecelerationRate = DecelerationRate;

		if (DecelerationCurve)
		{
			ActualDecelerationRate *= CalculateDecelerationCurveMultiplier(DecelerationCurve);
		}

		// Gradually decrease the max speed limit
		CurrentMaxSpeedLimit = FMath::Max(
			CurrentMaxSpeedLimit - ActualDecelerationRate * GetWorld()->GetDeltaSeconds(),
			0.0f
		);

		if (AbsSpeed > KINDA_SMALL_NUMBER)
		{
			UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetPawn()->GetRootComponent());

			if (!Root || !Root->IsSimulatingPhysics())
				return;

			// Only modify velocity if we're above our new speed limit
			if (AbsSpeed > CurrentMaxSpeedLimit)
			{
				FVector ForwardDir = GetPawn()->GetActorForwardVector().GetSafeNormal();

				// Preserve sign (forward or reverse)
				float ClampedSpeed = FMath::Sign(CurrentForwardSpeed) * CurrentMaxSpeedLimit;

				FVector NewVel = ForwardDir * ClampedSpeed;

				// Preserves vertical velocity
				FVector CurrentVel = Root->GetPhysicsLinearVelocity();
				NewVel.Z = CurrentVel.Z;

				Root->SetPhysicsLinearVelocity(NewVel, false);
			}
		}
	}
	else
	{
		// When input is detected, reset the current max speed limit to the base value
		CurrentMaxSpeedLimit = BaseMaxSpeed;
	}
}

void ATankController::RefreshTankPlayerState()
{
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
}

void ATankController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RefreshTankPlayerState();
    ClampVehicleSpeed();
    HandleVehicleDeceleration();
}

void ATankController::UpdateTickEnable(const bool bEnable)
{
	SetActorTickEnabled(bEnable);
}

void ATankController::SetDefaults()
{
	UpdateTickEnable(true);
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

	SetDefaults();
}

void ATankController::AddInputMappingContext() const
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		if (!Subsystem->HasMappingContext(DefaultMappingContext))
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void ATankController::SetupInput()
{
	AddInputMappingContext();

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
	UpdateTickEnable(false);
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

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATankController::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATankController::Aim);

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
	return bInputMasterSwitch && bIsAlive && TankPlayer && VehicleMovementComponent && GetPawn();
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

	SetDriveTorque(LeftPower * MaxTorquePerWheel, RightPower * MaxTorquePerWheel);
		
	ClampVehicleSpeed();
	HandleVehicleDeceleration();
}

void ATankController::Move(const FInputActionValue& Value)
{
	if (!CanRegisterInput())
		return;

	bIsInAir = !TankPlayer->IsInAir();
	SR_Move(Value.GetMagnitude());
}

void ATankController::SR_Move_Implementation(double Value)
{
	MC_Move(Value);
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

void ATankController::Turn__Internal(double Value)
{
	MoveValues.X = Value;
	bIsInAir = !TankPlayer->IsInAir();
	
	VehicleMovementComponent->SetYawInput(MoveValues.X);

	ClampVehicleSpeed();
	HandleVehicleDeceleration();
}

void ATankController::MC_Turn_Implementation(const FInputActionValue& Value)
{
	Turn__Internal(Value.GetMagnitude());
}

void ATankController::SR_Turn_Implementation(const FInputActionValue& Value)
{
	MC_Turn(Value);
}

void ATankController::TurnCompleted__Internal()
{
	VehicleMovementComponent->SetThrottleInput(0);
	VehicleMovementComponent->SetBrakeInput(0);
	VehicleMovementComponent->SetYawInput(0);
	
	PrevTurnInput = 0; // fixes a bug. keep it
	MoveValues.X = 0;
}

void ATankController::SR_TurnCompleted_Implementation(const FInputActionValue& Value)
{
	MC_TurnCompleted(Value);
}

void ATankController::MC_TurnCompleted_Implementation(const FInputActionValue& Value)
{
	TurnCompleted__Internal();
}

void ATankController::Turn(const FInputActionValue& Value)
{
	if (!CanRegisterInput())
		return;

	Turn__Internal(Value.GetMagnitude());
	SR_Turn(Value);
}

void ATankController::TurnCompleted(const FInputActionValue& Value)
{
	if (!CanRegisterInput())
		return;
	
	TurnCompleted__Internal();
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

void ATankController::Aim(const FInputActionValue& InputActionValue)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::Aim)")),
										  true, true, FLinearColor::Yellow, 0);

	// add logic that would reset the lock and give a few seconds of grace time
}

void ATankController::SelfDestruct(const FInputActionValue& InputActionValue)
{
	if (!CanRegisterInput())
		return;
	
	if (UTankHealthComponent* e = TankPlayer->GetHealthComponent())
		e->SelfDestruct(-1);
}

void ATankController::MouseWheelUp(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;

	if (!TankPlayer->GetBackSpringArmComp() || !TankPlayer->GetFrontCameraComp())
		return;

	USpringArmComponent* BackSpringArmComp = TankPlayer->GetBackSpringArmComp();

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

	USpringArmComponent* BackSpringArmComp = TankPlayer->GetBackSpringArmComp();

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