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

const FName FirstPersonSocket = FName("FirstPersonSocket");

ATankController::ATankController(): LookValues(), MoveValues(), bCanMove(true), ShootTimerDuration(3), MouseSensitivity(FVector(0.4)),
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

	for (auto Element : GetComponents())
	{
		Element->SetIsReplicated(true);
	}
}

void ATankController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TankPlayer)
	{
		bStopTurn = TankPlayer->GetMesh()->GetPhysicsAngularVelocityInDegrees().Length() > 30.0;
		TankPlayer->SetSpeed(TankPlayer->GetVehicleMovementComponent()->GetForwardSpeed());
	}
}

void ATankController::SetDefaults()
{
	TankPlayer = Cast<ATankCharacter>(GetPawn());
	// checkf(TankPlayer, TEXT("TankPlayer is invalid in ATankController::SetDefaults"));
	
	if (TankPlayer)
		if (TankPlayer->GetBackSpringArmComp()->TargetArmLength == TankPlayer->GetMaxZoomIn())
			TankPlayer->bAimingIn = true;
		else if (TankPlayer->GetBackSpringArmComp()->TargetArmLength > TankPlayer->GetMaxZoomIn())
			TankPlayer->bAimingIn = false;
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
	ensure(EnhancedInputComponent);
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
	if (!TankPlayer)
		return;
	
	// input is a Vector2D
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
}

void ATankController::Look(const FInputActionValue& Value)
{
	if (!TankPlayer)
		return;
	
	// input is a Vector2D
	LookValues = Value.Get<FVector2D>() * MouseSensitivity;

	// add yaw and pitch input to controller
	AddYawInput(LookValues.X);
	AddPitchInput(LookValues.Y);
}

void ATankController::Turn(const FInputActionValue& Value)
{
	if (!TankPlayer)
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
	if (!TankPlayer)
		return;
	TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0.1);
}

void ATankController::TurnCompleted(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;
	TankPlayer->GetVehicleMovementComponent()->SetThrottleInput(0);
	TankPlayer->GetVehicleMovementComponent()->SetBrakeInput(0);
}

void ATankController::SpawnShootEmitters() const
{
	for (auto ParticleSystem : TankPlayer->GetShootEmitterSystems())
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, TankPlayer->GetShootSocket()->GetComponentTransform());
}

void ATankController::SR_SpawnShootEmitters_Implementation()
{
	MC_SpawnShootEmitters();
}

void ATankController::MC_SpawnShootEmitters_Implementation()
{
	SpawnShootEmitters();
}

void ATankController::SpawnHitParticleSystem(const FVector& Location)
{
	UGameplayStatics::SpawnEmitterAtLocation( GetWorld(),
		TankPlayer->GetShootHitParticleSystem(), Location);
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

AActor* ATankController::FindClosestHighlightedActor() const
{
	AActor* ClosestActor = nullptr;
	for (const auto& Hit : TankPlayer->GetHighlightedEnemyTanks())
		if (ClosestActor == nullptr)
			ClosestActor = Hit.GetActor(); // just sets the first actor in the array as ClosestActor
		else if (ClosestActor != Hit.GetActor() && TankPlayer->GetDistanceTo(Hit.GetActor()) < TankPlayer->GetDistanceTo(ClosestActor))
			ClosestActor = Hit.GetActor(); // tries to find the closest actor to the player
	return ClosestActor;
}

void ATankController::Shoot(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;
	
	if (!TankPlayer->GetShootSocket())
		return;

	if (bCanShoot == false || bShootingBlocked == true)
		return;
	
	constexpr double ShootTraceDistance = 15000.0;

	// disable shooting
	SetCanShoot(false);
	StartShootTimer();

	// Spawning muzzle fire and dust around the tank 
	SR_SpawnShootEmitters();
	auto TurretStart = TankPlayer->GetMesh()->GetSocketLocation("GunShootSocket");
	FVector End = TurretStart + TankPlayer->GetMesh()->GetSocketQuaternion("GunShootSocket").GetForwardVector() * ShootTraceDistance;

	FHitResult OutHit;
	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TurretStart,
		End,
		TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForDuration,
		OutHit,
		true,
		FLinearColor::Black
	);

	// check if trace hit something
	if (bHit)
	{
		for (const auto& Element : TankPlayer->GetHighlightedEnemyTanks()) // check if the hit actor is in the highlighted enemy tanks
		{
			if (Element.GetActor() == OutHit.GetActor())
			{
				// if it does exist in the array, spawn the hit particle system and return.
				SpawnHitParticleSystem(OutHit.Location);
				return;
			}
		}
	}

	// if it either did not hit anything or the hit actor is not in the highlighted enemy tanks array,
	// then we try to find the closest location where we can shoot to actually hit the tank.
	if (TankPlayer->GetHighlightedEnemyTanks().IsEmpty())
		return;
	
	// finds closest actor to player
	AActor* ClosestHighlightedActor = FindClosestHighlightedActor();

	if (ClosestHighlightedActor == nullptr)
		return;
	
	auto ActorSkeletalMesh = ClosestHighlightedActor->GetComponentByClass<USkeletalMeshComponent>();

	if (ActorSkeletalMesh == nullptr)
		return;

	// get geometric center of the enemy tank
	auto ActorCenter = ActorSkeletalMesh->Bounds.GetBox().GetCenter();

	FVector2D ScreenPosition;
	ProjectWorldLocationToScreen(ActorCenter, ScreenPosition); // find where the geometric center of the tank is on screen

	for (int i = 0; i <= 10; ++i)
	{
		auto Lerp = FMath::Lerp(TankPlayer->GunTraceScreenPosition, ScreenPosition, i * 0.1);

		FVector WorldLocation;
		FVector WorldDirection;
		DeprojectScreenPositionToWorld(Lerp.X, Lerp.Y, WorldLocation, WorldDirection);

		FHitResult LerpHit;
		bool bLerpHit = UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),
			TurretStart,
			WorldLocation + WorldDirection * ShootTraceDistance,
			TraceTypeQuery1,
			false,
			{},
			EDrawDebugTrace::ForDuration,
			LerpHit,
			true,
			FLinearColor::Red
		);

		if (bLerpHit)
			if (ClosestHighlightedActor == LerpHit.GetActor())
				SpawnHitParticleSystem(LerpHit.Location);
	}
}

void ATankController::HandbrakeStarted(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;
	
	TankPlayer->GetVehicleMovementComponent()->SetHandbrakeInput(true);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandbrakeStarted)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATankController::HandbrakeEnded(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
    		return;
    		
	TankPlayer->GetVehicleMovementComponent()->SetHandbrakeInput(false);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankController::HandbrakeEnded)")),
		true, true, FLinearColor::Yellow, 0);
}

void ATankController::MouseWheelUp(const FInputActionValue& InputActionValue)
{
	if (!TankPlayer)
		return;
	
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
	if (!TankPlayer)
		return;
	
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
