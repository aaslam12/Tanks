// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedCodeFlow.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TankController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/TankAimAssistComponent.h"
#include "Components/TankHealthComponent.h"
#include "Components/TankHighlightingComponent.h"
#include "Components/TankPowerUpManagerComponent.h"
#include "Components/TankTargetingSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/TankGameState.h"
#include "GameFramework/TankPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Projectiles/ProjectilePool.h"
#include "Projectiles/TankDamageType.h"
#include "Projectiles/TankProjectile.h"
#include "Tanks/Public/Animation/TankAnimInstance.h"
#include "UI/WB_GunSight.h"

static int FriendStencilValue = 2;
static int EnemyStencilValue = 1;

ATankCharacter::ATankCharacter(): TankHighlightingComponent(CreateDefaultSubobject<UTankHighlightingComponent>("TankHighlightingComponent")),
								  TankPowerUpManagerComponent(CreateDefaultSubobject<UTankPowerUpManagerComponent>("TankPowerUpManagerComponent")),
								  TankAimAssistComponent(CreateDefaultSubobject<UTankAimAssistComponent>("TankAimAssistComponent")),
								  TankTargetingSystem(CreateDefaultSubobject<UTankTargetingSystem>("TankTargetingSystem")),
								  RadialForceComponent(CreateDefaultSubobject<URadialForceComponent>("RadialForceComponent")),
								  DamagedStaticMesh(CreateDefaultSubobject<UStaticMeshComponent>("Damaged Tank Mesh")),
								  MaxZoomIn(500), MaxZoomOut(2500), BasePitchMin(-20.0), BasePitchMax(10.0),
                                  AbsoluteMinGunElevation(-5), AbsoluteMaxGunElevation(30), TurretRotationSpeed(200),
                                  AimingTurretRotationSpeed(90), GunElevationInterpSpeed(10), BaseDamage(500),
                                  MinGunElevation(-15), MaxGunElevation(20), GunElevation(0), CurrentTurretAngle(0),
                                  bIsInAir(false), 
                                  DesiredGunElevation(0), 
                                  LookValues(), MoveValues(), bAimingIn(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MiddleCameraComp = CreateDefaultSubobject<UCameraComponent>("MiddleCameraComp");
	MiddleSpringArmComp = CreateDefaultSubobject<USpringArmComponent>("MiddleSpringArmComp");

	MiddleSpringArmComp->SetupAttachment(GetMesh(), FName("TurretSocket"));
	MiddleCameraComp->SetupAttachment(MiddleSpringArmComp);
	MiddleCameraComp->SetActive(false);

	// RadialForceComponent->SetupAttachment(RootComponent, "Muzzle");
	DamagedStaticMesh->SetHiddenInGame(false);
	DamagedStaticMesh->SetVisibility(true);

	if (GetMesh())
	{
		// Enable custom depth for the tank mesh
		GetMesh()->SetRenderCustomDepth(true);
		// Set the custom depth stencil value to differentiate between different types of objects
		GetMesh()->SetCustomDepthStencilValue(0);
		
		GetMesh()->SetHiddenInGame(false);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);

		if (TankAnimInstanceClass)
			GetMesh()->SetAnimInstanceClass(TankAnimInstanceClass);
	}

	if (DamagedStaticMesh)
	{
		DamagedStaticMesh->SetupAttachment(GetRootComponent());
		DamagedStaticMesh->SetHiddenInGame(true);
		DamagedStaticMesh->SetSimulatePhysics(false);
		DamagedStaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}

	for (auto Element : GetComponents())
		Element->SetIsReplicated(true);
}

ATankCharacter::~ATankCharacter() {}

void ATankCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ShootSocket = GetShootSocketFromBP();

	FrontCameraComp = GetFrontCameraFromBP();
	FrontSpringArmComp = GetFrontSpringArmFromBP();

	BackCameraComp = GetBackCameraFromBP();
	BackSpringArmComp = GetBackSpringArmFromBP();

	if (BackSpringArmComp)
		BackSpringArmComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("CameraSocket"));

	if (FrontSpringArmComp)
		FrontSpringArmComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("FirstPersonSocket"));

	if (RadialForceComponent)
		RadialForceComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Muzzle"));

}

void ATankCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentTurretAngle);
	DOREPLIFETIME(ThisClass, CurrentTeam);
	DOREPLIFETIME(ThisClass, PlayerName);
}

void ATankCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	SetDefaults();

	DamagedStaticMesh->SetHiddenInGame(true);
	DamagedStaticMesh->SetVisibility(false);
}

void ATankCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocallyControlled())
	{
		// Destroy post-process component if not locally owned
		if (TankPostProcessVolume)
		{
			TankPostProcessVolume->DestroyComponent();
			TankPostProcessVolume = nullptr;
		}
	}
	
	ResetCameraRotation();
	SetDefaults();
	BindDelegates();

	DamagedStaticMesh->SetHiddenInGame(true);
	DamagedStaticMesh->SetVisibility(false);
}

void ATankCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (PlayerController)
	{
		if (!PlayerController->OnShoot.IsBound())
			PlayerController->OnShoot.Clear();
	}

	if (HealthComponent)
	{
		if (!HealthComponent->OnHealthChanged.IsBound())
			HealthComponent->OnHealthChanged.Clear();
		
		if (!HealthComponent->OnDie.IsBound())
			HealthComponent->OnDie.Clear();
	}
}

void ATankCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld())
		return;

	// Only run trace logic for the local player or server, not unnecessary clients
	if (HasAuthority() || IsLocallyControlled())
	{
		if (PlayerController)
		{
			MoveValues = PlayerController->GetMoveValues();
			LookValues = PlayerController->GetLookValues();
		}

		if (HealthComponent)
			if (HealthComponent->IsDead())
				return;

		if (LockedTarget == nullptr)
		{
			TurretTraceTick();
			UpdateTurretTurning(DeltaTime);
			UpdateGunElevation(DeltaTime);
			CheckIfGunCanLowerElevationTick(DeltaTime);
			UpdateCameraPitchLimits();
		}

		ConeTraceTick();
		TankAimAssistComponent->AimAssist(LockedTarget);
		CL_UpdateGunSightPosition();
	}
}

void ATankCharacter::TurretTraceTick_Implementation()
{
	constexpr double ShootTraceDistance = 15000.0;

	TurretStart = GetMesh()->GetSocketLocation("GunShootSocket");
	TurretEnd = TurretStart + GetMesh()->GetSocketQuaternion("GunShootSocket").GetForwardVector() * ShootTraceDistance;
	
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TurretStart,
		TurretEnd,
		TraceTypeQuery1,
		false,
		{this},
		bShowDebugTracesForTurret ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		TurretTraceHit,
		true,
		FLinearColor::Black
	);
}

void ATankCharacter::SetWheelIndices()
{
	auto* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!MoveComp) return;

	USkeletalMeshComponent* SkelMesh = GetMesh();
	check(SkelMesh);

	LeftWheelIndices.Reset();
	RightWheelIndices.Reset();
	
	const FTransform& RootTM = SkelMesh->GetComponentTransform();

	// WheelSetups is the array you configured in the component (one per bone)
	for (int32 i = 0; i < MoveComp->WheelSetups.Num(); ++i)
	{
		const FChaosWheelSetup& Setup = MoveComp->WheelSetups[i];
		// Get bone world location:
		FVector BoneWorldLoc = SkelMesh->GetBoneLocation(Setup.BoneName);
		// Convert to local vehicle space:
		FVector LocalLoc = RootTM.InverseTransformPosition(BoneWorldLoc);

		// Y > 0 = right side, Y < 0 = left side (UE uses X forward, Y right)
		if (LocalLoc.Y > 0.f)
			RightWheelIndices.Add(i);
		else
			LeftWheelIndices.Add(i);
	}
}

void ATankCharacter::SetDefaults_Implementation()
{
	SetActorScale3D(FVector(0.95));
	AnimInstance = Cast<UTankAnimInstance>(GetMesh()->GetAnimInstance());

	PlayerController = Cast<ATankController>(GetController());
	// PlayerController->Possess(this);
	TankHighlightingComponent->SetDefaults();

	if (GetPlayerState())
		if (Cast<ATankPlayerState>(GetPlayerState()))
			TankPlayerState = Cast<ATankPlayerState>(GetPlayerState());

	// stops the player from looking under the tank and above too much.
	if (PlayerController)
	{
		PlayerController->PlayerCameraManager->ViewPitchMin = -60.0;
		PlayerController->PlayerCameraManager->ViewPitchMax = 30.0;
	}

	if (FrontCameraComp)
		FrontCameraComp->SetActive(false);

	if (BackCameraComp)
		BackCameraComp->SetActive(true);

	if (DamagedStaticMesh)
		DamagedStaticMesh->SetWorldTransform(FTransform(FRotator(0), FVector(0, 0, -100000)));

	SetLightsEmissivity(0);

	if (GetPlayerState())
		if (Cast<ATankPlayerState>(GetPlayerState()))
			PlayerName = Cast<ATankPlayerState>(GetPlayerState())->CustomPlayerName;

	// Config.StartRadius = 45;
	// DistanceExponent = 1.5;
	// Config.EndRadiusExponent = 1;
	// Config.ConeLengthExponent = 1.320883;

	ImpulseStrengthExponent = 1.2;

	SetWheelIndices();

	VisibilityTraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	Hits.Empty(10);
}

void ATankCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// cache the EnhancedInputComponent to bind controls later in BeginPlay after the controller is available.
	if (PlayerInputComponent)
		if (Cast<UEnhancedInputComponent>(PlayerInputComponent))
			EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
}

void ATankCharacter::BindDelegates()
{
	if (PlayerController)
	{
		if (!PlayerController->OnShoot.IsAlreadyBound(this, &ATankCharacter::OnShoot))
			PlayerController->OnShoot.AddDynamic(this, &ATankCharacter::OnShoot);
	}

	if (HealthComponent)
	{
		if (!HealthComponent->OnHealthChanged.IsAlreadyBound(this, &ATankCharacter::OnHealthChanged))
			HealthComponent->OnHealthChanged.AddDynamic(this, &ATankCharacter::OnHealthChanged);
		
		if (!HealthComponent->OnDie.IsAlreadyBound(this, &ATankCharacter::OnDie))
			HealthComponent->OnDie.AddDynamic(this, &ATankCharacter::OnDie);
	}
}

void ATankCharacter::ResetCameraRotation_Implementation()
{
    if (!BackCameraComp || !BackSpringArmComp || !PlayerController)
	    return;

	const FTransform OldTransform = GetActorTransform();
	
	PlayerController->SetControlRotation(GetActorForwardVector().Rotation());
	GunElevation = 0;
	CurrentTurretAngle = 0;

    FHitResult OutHit;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		GetActorLocation(),
		GetActorLocation() - FVector(0, 0, -2000),
		TraceTypeQuery1,
		false, {},
		EDrawDebugTrace::ForDuration,
		OutHit,
		true
	);

	SetActorTransform(
		FTransform(OldTransform.GetRotation(), OutHit.Location, OldTransform.GetScale3D()),
		false, nullptr,
		ETeleportType::ResetPhysics
	);

	PlayerController->bInputMasterSwitch = false;

	FFlow::Delay(this, 5.0, [this]
	{
		PlayerController->bInputMasterSwitch = true;
	});
}

void ATankCharacter::ConeTraceTick_Implementation()
{
	if (ConeTraceConfigs.IsEmpty() || bConeTraceDisabled)
		return;

	auto SkeletalMeshComponent = GetMesh();
	const FVector StartLocation = SkeletalMeshComponent->GetSocketLocation(FName("Muzzle"));
	const FVector Direction = SkeletalMeshComponent->GetSocketQuaternion(FName("Muzzle")).GetForwardVector();

	// this is how big we want the initial array to be.
	constexpr int SlackSize = 20;
	
	// Clear all previous hits before starting a new trace sequence
	AllHits.Empty(SlackSize);

	for (int i = 0; i < ConeTraceConfigs.Num(); ++i)
	{
		FConeTraceConfig& Config = ConeTraceConfigs[i];
		auto End = StartLocation + Direction * Config.ConeLength;

		bool bTraceBlockedByNonVehicle = false;

		for (int32 k = 0; k < Config.Steps; ++k)
		{
			// Prevent division by zero when Config.Steps is 1
			float Alpha = Config.Steps > 1 ? (float) k / (Config.Steps - 1) : 0;
			
			FVector SweepCenter = FMath::Lerp(
				StartLocation,
				End, Alpha == 1 || Alpha == 0 ? Alpha : FMath::Pow(Alpha, Config.CenterExponent)
			);
			
			float Radius = FMath::Lerp(Config.StartRadius, Config.EndRadius, Alpha);

			const auto bHit = UKismetSystemLibrary::SphereTraceMulti(
				GetWorld(),
				SweepCenter,
				SweepCenter,
				Radius,
				VisibilityTraceType,
				false,
				{this},
				bShowDebugTracesForTurret ? Config.DrawDebugTrace.GetValue() : EDrawDebugTrace::None,
				Hits,
				true,
				Config.ConeTraceColor,
				Config.ConeTraceHitColor
			);

			// Process hits for tank targeting if this config is used for targeting and if the array is not empty
			if (Config.bIsUsedForTankTargeting && Hits.IsEmpty() == false)
				for (const FHitResult& Hit : Hits)
					if (Hit.GetActor()->GetRootComponent()->GetCollisionObjectType() == ECC_Vehicle)
						AllHits.Add(Hit.GetActor());

			// If we hit anything, check if we should break the trace sequence
			if (bHit)
			{
				// Check for non-vehicle objects that would block the trace
				for (const FHitResult& Hit : Hits)
				{
					if (Hit.GetActor()->GetRootComponent()->GetCollisionObjectType() != ECC_Vehicle)
					{
						bTraceBlockedByNonVehicle = true;
						break;
					}
				}

				if (bTraceBlockedByNonVehicle)
					break; // Stop doing traces for this config as something non-vehicle is blocking
			}
		}

		// Process Hits as needed for this config
		if (Config.bIsUsedForTankTargeting && TankTargetingSystem)
		{
			LockedTarget = TankTargetingSystem->ProcessHitResults(AllHits);
			AllHits.Empty(SlackSize); // Clear hits after processing for this config
		}
	}
}

bool ATankCharacter::IsEnemy(AActor* OtherActor) const
{
	return CurrentTeam != Execute_GetCurrentTeam(OtherActor);
}

void ATankCharacter::HandleTakeDamage_Implementation(float DamageAmount, class AController* EventInstigator, AActor* DamageCauser)
{
	if (IsEnemy(DamageCauser)) // add a IsFriendlyFireOn toggle here
		HealthComponent->OnDamaged(this, DamageAmount, nullptr, EventInstigator, DamageCauser);
}

float ATankCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                 class AController* EventInstigator, AActor* DamageCauser)
{
	double ActualDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	HandleTakeDamage(ActualDamageAmount, EventInstigator, DamageCauser);

	return ActualDamageAmount;
}

void ATankCharacter::UpdateTurretTurning_Implementation(float DeltaTime)
{
	if (!Controller || !GetMesh() || !AnimInstance)
		return;

	if (bAimingIn)
	{
		// First person turret rotation
		float DeltaAngle = FMath::Clamp(LookValues.X, -AimingTurretRotationSpeed / 2, AimingTurretRotationSpeed / 2);

		// Clamp the angle difference based on MaxTurretRotationSpeed
		const double MaxDeltaAngle = TurretRotationSpeed * DeltaTime;
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);
		
		CurrentTurretAngle += DeltaAngle;

		SetTurretRotation(CurrentTurretAngle);
	}
	else if (LockedTarget == nullptr)
	{
		// 3rd person turret rotation
		FVector PlayerViewPointLocation;
		FRotator PlayerViewPointRotation;
		Controller->GetPlayerViewPoint(PlayerViewPointLocation, PlayerViewPointRotation);

		// Calculate the target direction
		FVector TurretToLookDir = PlayerViewPointRotation.Vector();
		TurretToLookDir.Z = 0.f;
		if (!TurretToLookDir.IsNearlyZero())
			TurretToLookDir.Normalize();

		FVector TurretForwardVector = GetMesh()->GetSocketQuaternion("t").GetForwardVector();
		TurretForwardVector.Z = 0.f;
		if (!TurretForwardVector.IsNearlyZero())
			TurretForwardVector.Normalize();

		// Calculate the target angle
		double DotProduct = FVector::DotProduct(TurretForwardVector, TurretToLookDir);

		// DO NOT CHANGE TOLERANCE (0.008 also works ig. idk which value is better)
		constexpr double Tolerance = 0.008; // setting it to 0.01 fixed it now somehow when it wasn't working before. DO NOT CHANGE
		if (FMath::IsNearlyEqual(DotProduct, 1.0f, Tolerance))
			DotProduct = 1.f; // Prevent any small rounding errors
		else if (FMath::IsNearlyEqual(DotProduct, -1.0f, Tolerance))
			DotProduct = -1.f; // Handle opposite direction

		double Det = FVector::CrossProduct(TurretForwardVector, TurretToLookDir).Z;
		if (FMath::IsNearlyZero(Det, Tolerance))
			Det = 0.f;

		double TargetAngle = FMath::RadiansToDegrees(FMath::Atan2(Det, DotProduct));
		if (FMath::IsNearlyEqual(TargetAngle, 1.0, Tolerance))
			TargetAngle = 1;

		// Calculate the *difference* in angle, but now wrap it to the shortest path
		double DeltaAngle = UKismetMathLibrary::NormalizeAxis(TargetAngle - AnimInstance->TurretAngle);

		// Clamp the angle difference based on MaxTurretRotationSpeed
		const double MaxDeltaAngle = TurretRotationSpeed * DeltaTime;
		DeltaAngle = FMath::Clamp(DeltaAngle, -MaxDeltaAngle, MaxDeltaAngle);

		// Update the turret angle
		SetTurretRotation(AnimInstance->TurretAngle + DeltaAngle);

		// UKismetSystemLibrary::PrintString(
		// 	  GetWorld(), 
		// 	  FString::Printf(TEXT("(ATankCharacter::UpdateTurretTurning) DeltaAngle: %.3f"), DeltaAngle), 
		// 	  true, 
		// 	  true, 
		// 	  FLinearColor::Green, 
		// 	  0
		// );
		//
		// UKismetSystemLibrary::PrintString(
		// 	  GetWorld(), 
		// 	  FString::Printf(TEXT("(ATankCharacter::UpdateTurretTurning) MaxDeltaAngle: %.3f"), MaxDeltaAngle), 
		// 	  true, 
		// 	  true, 
		// 	  FLinearColor::Green, 
		// 	  0
		// );
	}
}

void ATankCharacter::UpdateDesiredTurretAngle()
{
	SetDesiredTurretAngle(FMath::FInterpTo(CurrentTurretAngle, DesiredTurretAngle_C, GetWorld()->GetDeltaSeconds(), 10));
}

void ATankCharacter::CL_UpdateGunSightPosition_Implementation()
{
	if (IsAimingIn())
		return;

	if (GunSightWidget)
	{
		FVector2D ScreenPosition;
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PlayerController,
			TurretTraceHit.bBlockingHit ? TurretTraceHit.ImpactPoint : TurretTraceHit.TraceEnd,
			ScreenPosition,
			true
		);

		GunSightWidget->ScreenPosition = ScreenPosition;
	}
}

void ATankCharacter::SetDesiredTurretAngle(float TurretAngle)
{
	DesiredTurretAngle_C = TurretAngle;
}

void ATankCharacter::CheckIfGunCanLowerElevationTick_Implementation(float DeltaTime)
{
	if (!PlayerController)
		return;

	double OldMinTurretElevation = MinGunElevation;
	
	FVector TopTraceStart = GetMesh()->GetSocketLocation("BarrelTraceStart");
	FVector TopTraceEnd = GetMesh()->GetSocketLocation("BarrelTraceEnd");
	FHitResult TopHit;

	// Perform a trace along the bottom of the barrel to check if it's colliding with anything
	const bool bTopHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TopTraceStart,
		TopTraceEnd,
		TraceTypeQuery1,
		false,
		{},
		bShowDebugTracesForTurret ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		TopHit,
		false
	);

	FVector BottomTraceStart = GetMesh()->GetSocketLocation("BarrelTrace2Start");
	FVector BottomTraceEnd = GetMesh()->GetSocketLocation("BarrelTrace2End");
	FHitResult BottomHit;

	// Perform a trace along the bottom of the barrel to check if it's colliding with anything
	const bool bBottomHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		BottomTraceStart,
		BottomTraceEnd,
		TraceTypeQuery1,
		false,
		{},
		bShowDebugTracesForTurret ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		BottomHit,
		false
	);

	// disable the players ability to shoot while the turret is adjusting
	if (bBottomHit == true && bTopHit == false || bBottomHit == true && bTopHit == true)
		PlayerController->SetShootingBlocked(false);

	if (bTopHit || bBottomHit) 
	{
		
		if (!TopHit.PhysMaterial.IsValid() || !BottomHit.PhysMaterial.IsValid())
			return;

		auto bTopDetectTank = TopHit.PhysMaterial->SurfaceType == SurfaceType2 && TopHit.GetActor() == this;
		auto bBottomDetectTank = BottomHit.PhysMaterial->SurfaceType == SurfaceType2 && BottomHit.GetActor() == this;

		if (bAimingIn)
		{
			// MinGunElevation = AbsoluteMinGunElevation;
			
			if (bBottomDetectTank == true && bTopDetectTank == true)
			{
				MinGunElevation = FMath::Clamp(MinGunElevation + 1, MinGunElevation, AbsoluteMaxGunElevation); // 17
				PlayerController->SetShootingBlocked(true);
			}
			else if (bBottomDetectTank == true && bTopDetectTank == false)
			{
				MinGunElevation = GunElevation;
				PlayerController->SetShootingBlocked(false);
			}
			
		}
		else // 3rd person
		{
			if (bTopDetectTank == true) // if tank body physical material is detected and is the same actor
			{
				MinGunElevation = FMath::Clamp(
					FMath::Max(OldMinTurretElevation, MinGunElevation + (MaxTurretElevationAdjustSpeed * DeltaTime)),
					MinGunElevation,
					AbsoluteMaxGunElevation
				);
				
				PlayerController->SetShootingBlocked(true);
			}

			if (bBottomDetectTank == true && bTopDetectTank == false)
			{
				MinGunElevation = GunElevation;
			}
		}
	}
	else
	{
		// Update the last free gun elevation
		MinGunElevation = FMath::Clamp(MinGunElevation - (MaxTurretElevationAdjustSpeed * DeltaTime), AbsoluteMinGunElevation, GunElevation);

		PlayerController->SetShootingBlocked(false);
	}

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(CheckIfGunCanLowerElevationTick) OldTurretElevation: (%.3f/%.3f)"), OldMinTurretElevation, AbsoluteMinGunElevation),
	// 									  true, true, FLinearColor::Yellow, 0);
	//
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(CheckIfGunCanLowerElevationTick) MinGunElevation: (%.3f/%.3f)"), MinGunElevation, AbsoluteMinGunElevation),
	// 									  true, true, FLinearColor::Yellow, 0);
}

void ATankCharacter::UpdateGunElevation_Implementation(float DeltaTime)
{
	if (!BackCameraComp && LockedTarget == nullptr)
		return;

	if (bAimingIn)
	{
		GunElevation += LookValues.Y * -1;
		GunElevation = FMath::Clamp(GunElevation, MinGunElevation, MaxGunElevation);
		SetGunElevation(GunElevation);
		return;
	}

	auto ActiveCamera = GetActiveCamera();
	if (ActiveCamera == nullptr)
		return;
	
	ActiveCameraStart = ActiveCamera->GetComponentLocation();
	ActiveCameraEnd = ActiveCameraStart + (ActiveCamera->GetForwardVector() * 15000.0);
	
	auto bHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		ActiveCameraStart,
		ActiveCameraEnd,
		TraceTypeQuery1, // should be visibility
		false,
		{this},
		bShowDebugTracesForTurret ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		CameraTraceHit,
		true
	);
	
	DesiredCameraImpactPoint = bHit ? CameraTraceHit.ImpactPoint : ActiveCameraEnd;
	CameraImpactPoint = FMath::VInterpTo(CameraImpactPoint, DesiredCameraImpactPoint, DeltaTime, 30);
	
	GunRotation = UKismetMathLibrary::FindLookAtRotation(
		TurretStart,
		CameraImpactPoint
	);
	
	DesiredGunElevation = GunRotation.Pitch;
	
	GunElevation = FMath::Clamp(
		FMath::FInterpTo(GunElevation, DesiredGunElevation, DeltaTime, GunElevationInterpSpeed),
		MinGunElevation,
		MaxGunElevation
	);
	
	SetGunElevation(GunElevation);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) Start: [%s]"), *ActiveCameraStart.ToString()),
									  true, true, FLinearColor::White, 0);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) End: [%s]"), *ActiveCameraEnd.ToString()),
									  true, true, FLinearColor::White, 0);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) Looking Impact Point: [%s]"), *CameraTraceHit.ImpactPoint.ToString()),
									  true, true, FLinearColor::Yellow, 0);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) Turret Impact Point: [%s]"), *CameraImpactPoint.ToString()),
									  true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) GunRotation: [%s]"), *GunRotation.ToString()),
									  true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) TankRotation: [%s]"), *GetActorRotation().ToString()),
									  true, true, FLinearColor::Yellow, 0);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) DesiredGunElevation: [%.3f]"), DesiredGunElevation),
									  true, true, FLinearColor::Yellow, 0);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) GunElevation: [%.3f]"), GunElevation),
									  true, true, FLinearColor::Yellow, 0);
}

void ATankCharacter::UpdateIsInAir_Implementation()
{
	// FVector ActorOrigin = GetActorLocation();
	//
	// FHitResult Hit;
	// bIsInAir = UKismetSystemLibrary::LineTraceSingle(
	// 	GetWorld(),
	// 	ActorOrigin + FVector(0, 0, 150),
	// 	ActorOrigin - FVector(0, 0, 40),
	// 	TraceTypeQuery1,
	// 	false, {this},
	//	bShowDebugTracesForTurret ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
	// 	Hit,
	// 	true,
	// 	FLinearColor::Red
	// );

	bIsInAir = false;
}

void ATankCharacter::UpdateCameraPitchLimits_Implementation() const
{
	if (!PlayerController)
		return;

	const FVector Forward = GetActorForwardVector();
	const FVector WorldUp = FVector(0.0f, 0.0f, 1.0f);
	
	// Calculate pitch angle offset based on forward slope
	float ForwardPitchOffset = FMath::RadiansToDegrees(FMath::Asin(FVector::DotProduct(Forward, WorldUp)));

	
	// Adjust pitch limits based on the tank's orientation on slopes
	float AdjustedMinPitch = BasePitchMin - ForwardPitchOffset;
	float AdjustedMaxPitch = BasePitchMax + ForwardPitchOffset;
	
	// Apply the adjusted limits
	PlayerController->PlayerCameraManager->ViewPitchMin = AdjustedMinPitch;
	PlayerController->PlayerCameraManager->ViewPitchMax = AdjustedMaxPitch;
}

void ATankCharacter::OutlineTank_Implementation(const bool bActivate, const bool bIsFriend)
{
	if (!GetMesh())
		return;
	
	if (bActivate)
	{
		if (bIsFriend)
		{
			if (GetMesh()->CustomDepthStencilValue == 0 && GetMesh()->CustomDepthStencilValue != FriendStencilValue)
			{
				GetMesh()->SetCustomDepthStencilValue(FriendStencilValue);
			}
		}
		else
		{
			if (GetMesh()->CustomDepthStencilValue != FriendStencilValue && GetMesh()->CustomDepthStencilValue != EnemyStencilValue)
			{
				GetMesh()->SetCustomDepthStencilValue(EnemyStencilValue);
			}
		}
	}
	else
	{
		if (GetMesh()->CustomDepthStencilValue == EnemyStencilValue && GetMesh()->CustomDepthStencilValue != 0)
		{
			GetMesh()->SetCustomDepthStencilValue(0);
		}
	}
}

ETeam ATankCharacter::GetCurrentTeam_Implementation()
{
	if (TankPlayerState)
		return TankPlayerState->GetCurrentTeam();
	return ETeam::Unassigned; // empty string
}

void ATankCharacter::PowerUpActivated_Implementation(const EPowerUpType PowerUpType)
{
}

void ATankCharacter::ProjectileHit_Implementation(ATankProjectile* TankProjectile, UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	IShootingInterface::ProjectileHit_Implementation(TankProjectile, HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	SR_ApplyRadialDamage(Hit);
	TankProjectile->ResetTransform();
}

void ATankCharacter::ApplyRadialImpulseToObjects_Implementation(const FHitResult& Hit)
{
	TArray<FHitResult> OutHits;
	double TraceRadius = 2500;

	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		Hit.Location,
		Hit.Location,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius)
	);
	
	if (bHit)
	{
		for (auto OutHit : OutHits)
		{
			// dont apply impulse to self here. will do this elsewhere.
			if (OutHit.GetActor() == this)
				continue;
			
			UPrimitiveComponent* HitComp = OutHit.GetComponent();

			if (HitComp && HitComp->IsSimulatingPhysics())
			{
				FVector ImpulseOrigin = OutHit.ImpactPoint;
				float ImpulseStrength = FMath::Pow(FMath::Pow(RadialForceComponent->ImpulseStrength, ImpulseStrengthExponent), 1.0f - (OutHit.Distance / TraceRadius));
				float ImpulseRadius = RadialForceComponent->Radius;

				HitComp->AddRadialImpulse(
					ImpulseOrigin,
					ImpulseRadius,
					ImpulseStrength,
					RIF_Linear,
					false // velocity change
				);
			}
		}
	}
}

void ATankCharacter::ApplyRadialDamage_Implementation(const FHitResult& Hit)
{
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(),
	                                   BaseDamage, BaseDamage * 0.1, Hit.Location,
	                                   DamageInnerRadius, DamageOuterRadius, DamageFalloffExponent, UTankDamageType::StaticClass(),
	                                   {}, this, GetController());

	ApplyRadialImpulseToObjects(Hit);
}

void ATankCharacter::SR_ApplyRadialDamage_Implementation(const FHitResult& Hit)
{
	MC_ApplyRadialDamage(Hit);
}

void ATankCharacter::MC_ApplyRadialDamage_Implementation(const FHitResult& Hit)
{
	ApplyRadialDamage(Hit);
}

void ATankCharacter::Restart()
{
	Super::Restart();

	if (!GetWorld()->HasBegunPlay())
		return;

	ResetCameraRotation();
	SR_Restart();
}

void ATankCharacter::SR_Restart_Implementation()
{
	MC_Restart();
}

void ATankCharacter::MC_Restart_Implementation()
{
	Restart__Internal();
}

void ATankCharacter::Restart__Internal()
{
	if (OnDieStaticMesh)
	{
		GetMesh()->SetHiddenInGame(false);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		
		RadialForceComponent->FireImpulse();
		DamagedStaticMesh->SetWorldTransform(FTransform(FRotator::ZeroRotator, FVector(0, 0, -100000))); // TODO. add a small impulse to the tank and bounce it up
		DamagedStaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		DamagedStaticMesh->SetHiddenInGame(true);
		DamagedStaticMesh->SetVisibility(false);
	}

	if (PlayerController)
		PlayerController->OnRespawn();

	SetDefaults();
	BindDelegates();

	if (HealthComponent)
		HealthComponent->OnPlayerRespawn();

	SetActorTickEnabled(true);
}

void ATankCharacter::OnDie_Implementation(APlayerState* AffectedPlayerState, bool bSelfDestruct, bool bShouldRespawn)
{
	if (OnDieStaticMesh)
	{
		GetMesh()->SetHiddenInGame(true);
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		
		RadialForceComponent->FireImpulse();
		DamagedStaticMesh->SetWorldTransform(GetActorTransform()); // TODO. add a small impulse to the tank and bounce it up
		DamagedStaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		DamagedStaticMesh->SetHiddenInGame(false);
		DamagedStaticMesh->SetVisibility(true);
	}

	if (PlayerController)
		PlayerController->OnDie();

	SetActorTickEnabled(false);
}

void ATankCharacter::OnHealthChanged_Implementation(float NewHealth, bool bIsRegenerating)
{
}

void ATankCharacter::ApplyTankShootImpulse_Implementation() const
{
	FVector TurretDirection = GetMesh()->GetSocketQuaternion("GunShootSocket").GetRightVector();
	TurretDirection.Normalize();
	FVector AngularImpulse = TurretDirection * -FMath::Abs(OnShootImpulseStrength); // Adjust multiplier for desired strength
	
	GetMesh()->AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);
}

void ATankCharacter::SpawnProjectileFromPool()
{
	auto GameMode = Cast<ATankGameState>(UGameplayStatics::GetGameState(GetWorld()));
		
	if (GameMode != nullptr)
	{
		if (GameMode->ProjectilePool != nullptr)
		{
			auto Rot = UKismetMathLibrary::FindLookAtRotation(TurretTraceHit.TraceStart, TurretTraceHit.TraceEnd);
			GameMode->ProjectilePool->SpawnFromPool(FTransform(Rot, TurretTraceHit.TraceEnd), this);
		}
	}
}

void ATankCharacter::OnShoot_Implementation()
{
	// Spawning muzzle fire and dust around the tank 
	SR_SpawnShootEmitters();

	// check if trace hit something. the trace is running on tick in another function
	if (TurretTraceHit.IsValidBlockingHit())
	{
		SpawnHitParticleSystem(TurretTraceHit);
		SR_ApplyRadialDamage(TurretTraceHit);
	}
	else
	{
		// spawn projectile for the rest of the way.
		SpawnProjectileFromPool();
	}

	ApplyTankShootImpulse();
}

void ATankCharacter::SetGunElevation(const double NewGunElevation) const
{
	if (AnimInstance == nullptr)
		return;
	
	if (HasAuthority())
		AnimInstance->GunElevation = NewGunElevation;
	else
		SR_SetGunElevation(NewGunElevation);
}

void ATankCharacter::SpawnHitParticleSystem(const FHitResult& Hit) const
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ShootHitParticleSystem,
		Hit.Location,
		Hit.Normal.Rotation(), // rotate to match hit surface
		FVector(1),
		true,
		true,
		ENCPoolMethod::AutoRelease
	);

	// DrawDebugSphere(GetWorld(),
	// 	Hit.Location, 75, 16,
	// 	FColor::Red, false,
	// 	bShowDebugTracesForTurret ? 5 : -1
	// );
	// UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ATankCharacter::SR_SetGunElevation_Implementation(double NewGunElevation) const
{
	if (AnimInstance == nullptr)
		return;

	AnimInstance->GunElevation = NewGunElevation;

	MC_SetGunElevation(NewGunElevation);
}

void ATankCharacter::MC_SetGunElevation_Implementation(double NewGunElevation) const
{
	if (AnimInstance)
		AnimInstance->GunElevation = NewGunElevation;
}

void ATankCharacter::SetTurretRotation(const double NewTurretAngle) const
{
	if (AnimInstance == nullptr)
		return;
	
	if (HasAuthority())
		AnimInstance->TurretAngle = NewTurretAngle;
	else
		SR_SetTurretRotation(NewTurretAngle);
}

void ATankCharacter::SR_SetTurretRotation_Implementation(double NewTurretAngle) const
{
	if (AnimInstance == nullptr)
		return;

	AnimInstance->TurretAngle = NewTurretAngle;
	MC_SetTurretRotation(NewTurretAngle);
}

void ATankCharacter::MC_SetTurretRotation_Implementation(double NewTurretAngle) const
{
	if (AnimInstance == nullptr)
		return;
	
	AnimInstance->TurretAngle = NewTurretAngle;
}

void ATankCharacter::SetSkinType(const double NewSkinType) const
{
	if (BodyMaterial)
		BodyMaterial->SetScalarParameterValue("SkinType", NewSkinType);
}

void ATankCharacter::SetLightsEmissivity(double LightsEmissivity) const
{
	if (BodyMaterial == nullptr)
		return;

	BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
	SR_SetLightsEmissivity(LightsEmissivity);
}

void ATankCharacter::SR_SetLightsEmissivity_Implementation(double LightsEmissivity) const
{
	if (BodyMaterial == nullptr)
		return;
	
	BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
	MC_SetLightsEmissivity(LightsEmissivity);
}

void ATankCharacter::MC_SetLightsEmissivity_Implementation(double LightsEmissivity) const
{
	if (BodyMaterial == nullptr)
		return;
	
	BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
}

void ATankCharacter::SetSpeed(double Speed)
{
	if (AnimInstance == nullptr)
		return;
	
	if (HasAuthority())
	{
		AnimInstance->WheelSpeed = Speed;
		SetWheelSmoke(!bIsInAir ? Speed : 0);
	}
	else
	{
		SR_SetSpeed(Speed);
	}
}

void ATankCharacter::SR_SetSpeed_Implementation(double Speed)
{
	MC_SetSpeed(Speed);
}

void ATankCharacter::MC_SetSpeed_Implementation(double Speed)
{
	if (AnimInstance == nullptr)
		return;
	
	AnimInstance->WheelSpeed = Speed;
	SetWheelSmoke(!bIsInAir ? Speed : 0);
}

void ATankCharacter::SetHatchesAngles(double HatchAngle) const
{
	if (AnimInstance)
		AnimInstance->HatchAngle = HatchAngle;
}

void ATankCharacter::SpawnShootEmitters() const
{
	const FTransform Transform = GetShootSocket()->GetComponentTransform();

	for (UParticleSystem* ParticleSystem : GetShootEmitterSystems())
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ParticleSystem,
			FTransform(
				Transform.GetRotation(),
				Transform.GetLocation(),
				FVector(0.5))
		);
	}

	RadialForceComponent->FireImpulse();
	
	GetMesh()->AddImpulseAtLocation(
		UKismetMathLibrary::FindLookAtRotation(
			GetMesh()->GetSocketLocation("gun_jnt"),
			GetActorLocation() + FVector(0, 0, 200)
		).Vector() * RadialForceComponent->ImpulseStrength * 10 + GetVehicleMovementComponent()->GetForwardSpeedMPH() * 1000,
		GetMesh()->GetSocketLocation("gun_jnt") 
	);
}

bool ATankCharacter::IsAimingIn() const
{
	if (BackSpringArmComp == nullptr)
		return false;
	if (BackSpringArmComp->TargetArmLength == MaxZoomIn)
		return true;
	if (BackSpringArmComp->TargetArmLength > MaxZoomIn)
		return false;
	return false;
}

const UCameraComponent* ATankCharacter::GetActiveCamera() const
{
	if (FrontCameraComp && FrontCameraComp->IsActive() && IsAimingIn())
		return FrontCameraComp;

	if (BackCameraComp && BackCameraComp->IsActive())
		return BackCameraComp;

	if (MiddleCameraComp && MiddleCameraComp->IsActive())
		return MiddleCameraComp;
	
	return nullptr;
}

const ATankController* ATankCharacter::GetPlayerController() const
{
	return PlayerController;
}

void ATankCharacter::MC_SpawnShootEmitters_Implementation()
{
	SpawnShootEmitters();
}

void ATankCharacter::SR_SpawnShootEmitters_Implementation()
{
	MC_SpawnShootEmitters();
}

void ATankCharacter::MC_SetWheelSmoke_Implementation(float Intensity)
{
	SetWheelSmoke(Intensity);
}

void ATankCharacter::MC_SetHatchesAngles_Implementation(double HatchAngle)
{
	SetHatchesAngles(HatchAngle);
}

bool ATankCharacter::CanLockOn() const
{
	return TankTargetingSystem ? TankTargetingSystem->CanLockOn() : false;
}

void ATankCharacter::SetCanLockOn(bool bCond) const
{
	if (TankTargetingSystem)
		TankTargetingSystem->SetCanLockOn(bCond);
}
