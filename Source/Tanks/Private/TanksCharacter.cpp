// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TanksCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosVehicleMovementComponent.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"


// Sets default values
ATanksCharacter::ATanksCharacter(): StopTurn(false), VehicleYaw(0)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// lf_wheel_01_jnt
	// lf_wheel_02_jnt
	// lf_wheel_03_jnt
	// lf_wheel_04_jnt
	// lf_wheel_05_jnt
	// lf_wheel_05_jnt
	// lf_wheel_06_jnt
	// lf_wheel_07_jnt
	// lf_wheel_08_jnt
	// lf_wheel_09_jnt

	// rt_wheel_01_jnt
	// rt_wheel_02_jnt
	// rt_wheel_03_jnt
	// rt_wheel_04_jnt
	// rt_wheel_05_jnt
	// rt_wheel_06_jnt
	// rt_wheel_07_jnt
	// rt_wheel_08_jnt
	// rt_wheel_09_jnt
}

void ATanksCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
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
	}
	else
	{
		UE_LOG(LogBlueprintUserMessages, Error,
			   TEXT(
				   "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
			   ), *GetNameSafe(this));
	}
}

// Called when the game starts or when spawned
void ATanksCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetActorScale3D(FVector(0.95));
	AnimInstance = CastChecked<UTankAnimInstance>(GetMesh()->GetAnimInstance());

	UGameplayStatics::GetPlayerController(GetWorld(), 0)->Possess(this);

	// stops the player from looking under the tank and above too much.
	UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->ViewPitchMin = -60.0;
	UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->ViewPitchMax = 15.0;
}

// Called every frame
void ATanksCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StopTurn = GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;

	SetSpeed(GetVehicleMovementComponent()->GetForwardSpeed());

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Move) MovementVector: %s"),
			*MoveValues.ToString()), true, true, FLinearColor::Yellow, 0);

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
		VehicleYaw = FMath::Lerp(VehicleYaw, 0.0, 0.1);
	else
		VehicleYaw = MoveValues.X;

	GetVehicleMovementComponent()->SetYawInput(VehicleYaw);
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
}

void ATanksCharacter::SetHatchesAngles(double HatchAngle)
{
	AnimInstance->HatchAngle = HatchAngle;
}
