// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedCodeFlow.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TankController.h"
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

static int FriendStencilValue = 2;
static int EnemyStencilValue = 1;

ATankCharacter::ATankCharacter(): TankHighlightingComponent(CreateDefaultSubobject<UTankHighlightingComponent>("TankHighlightingComponent")),
								  TankPowerUpManagerComponent(CreateDefaultSubobject<UTankPowerUpManagerComponent>("TankPowerUpManagerComponent")),
								  TankAimAssistComponent(CreateDefaultSubobject<UTankAimAssistComponent>("TankAimAssistComponent")),
								  TankTargetingSystem(CreateDefaultSubobject<UTankTargetingSystem>("TankTargetingSystem")),
								  RadialForceComponent(CreateDefaultSubobject<URadialForceComponent>("RadialForceComponent")),
								  DamagedStaticMesh(CreateDefaultSubobject<UStaticMeshComponent>("Damaged Tank Mesh")),
								  MaxZoomIn(500), MaxZoomOut(2500), BasePitchMin(-20.0), BasePitchMax(10.0),
                                  AbsoluteMinGunElevation(-5), AbsoluteMaxGunElevation(30), MaxTurretRotationSpeed(90),
                                  GunElevationInterpSpeed(10), BaseDamage(500),
                                  MinGunElevation(-15), MaxGunElevation(20), GunElevation(0), CurrentTurretAngle(0),
                                  bIsInAir(false), 
                                  DesiredGunElevation(0), 
                                  LookValues(), MoveValues(), bAimingIn(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

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

	ShootSocket = GetShootSocke();

	FrontCameraComp = GetFrontCamera();
	FrontSpringArmComp = GetFrontSpringArm();

	BackCameraComp = GetBackCamera();
	BackSpringArmComp = GetBackSpringArm();

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

// Called every frame
void ATankCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld())
		return;

	// Only run trace logic for the local player or server, not unnecessary clients
	if (HasAuthority() || IsLocallyControlled())
	{
		// If we're the server or the local player, run the traces and related logic
		if (PlayerController)
		{
			MoveValues = PlayerController->GetMoveValues();
			LookValues = PlayerController->GetLookValues();
		}

		if (HealthComponent)
			if (HealthComponent->IsDead())
				return;

		TurretTraceTick();
		ConeTraceTick(); // can be used if turret requires a cone trace for some reason. eg a fire turret
		UpdateTurretTurning(DeltaTime);
		UpdateGunElevation(DeltaTime);
		CheckIfGunCanLowerElevationTick(DeltaTime);
		UpdateCameraPitchLimits();
		TankAimAssistComponent->AimAssist(LockedTarget);

		// UpdateIsInAir();

		if (LockedTarget)
		{
			UpdateDesiredTurretAngle();
		}
	}
}

void ATankCharacter::TurretTraceTick_Implementation()
{
	constexpr double ShootTraceDistance = 15000.0;

	const FVector TurretStart = GetMesh()->GetSocketLocation("GunShootSocket");
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
	if (ConeTraceConfigs.IsEmpty())
		return;

	const FVector StartLocation = GetMesh()->GetSocketLocation("Muzzle");
	const FVector Direction = GetMesh()->GetSocketQuaternion("Muzzle").GetForwardVector();

	for (int i = 0; i < ConeTraceConfigs.Num(); ++i)
	{
		FConeTraceConfig& Config = ConeTraceConfigs[i];
		
		Config.EndRadius = FMath::Pow(Config.StartRadius * Config.Steps, Config.EndRadiusExponent);
		Config.ConeLength = FMath::Pow(FMath::Pow(Config.StartRadius * Config.Steps, Config.ConeLengthExponent), Config.EndRadiusExponent);
		
		for (int32 k = 0; k < Config.Steps; ++k)
		{
			float Alpha = (float)k / (float)(Config.Steps - 1);
			float Distance = FMath::Pow(Alpha, Config.DistanceExponent) * Config.ConeLength;
			FVector SweepCenter = StartLocation + Direction * Distance;
			float Radius = FMath::Lerp(Config.StartRadius, Config.EndRadius, Alpha);

			TArray<FHitResult> Hits;
			const auto bHit = UKismetSystemLibrary::SphereTraceMulti(
				GetWorld(),
				SweepCenter,
				SweepCenter,
				Radius,
				TraceTypeQuery1,
				false,
				{},
				bShowDebugTracesForTurret ? Config.DrawDebugTrace.GetValue() : EDrawDebugTrace::None,
				Hits,
				true,
				Config.ConeTraceColor,
				Config.ConeTraceHitColor
			);

			if (Config.bIsUsedForTankTargeting)
                for (const FHitResult& Hit : Hits)
                    if (Hit.GetActor()->GetRootComponent()->GetCollisionObjectType() == ECC_Vehicle)
                        AllHits.Add(Hit);
            
            if (bHit)
            {
                bool e = false;
                // if is not a vehicle, stop following traces.
                for (const FHitResult& Hit : Hits)
                {
                    if (Hit.GetActor()->GetRootComponent()->GetCollisionObjectType() != ECC_Vehicle)
                    {
                        e = true;
                        break;
                    }
                }
            
                if (e)
                    break;
            }
		}

		// Process Hits as needed
		if (Config.bIsUsedForTankTargeting)
		{
			LockedTarget = TankTargetingSystem->ProcessHitResults(AllHits);
			AllHits.Empty();
		}
	}
}

void ATankCharacter::HandleTakeDamage_Implementation(float DamageAmount, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Execute_GetCurrentTeam(this) != Execute_GetCurrentTeam(DamageCauser)) // add a IsFriendlyFireOn toggle here
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
		float DeltaTurretAngle = FMath::Clamp(LookValues.X, -MaxTurretRotationSpeed / 2, MaxTurretRotationSpeed / 2);
		CurrentTurretAngle += DeltaTurretAngle;

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

		FVector TurretForwardVector = GetMesh()->GetSocketQuaternion("turret_jntSocket").GetForwardVector();
		TurretForwardVector.Z = 0.f;
		if (!TurretForwardVector.IsNearlyZero())
			TurretForwardVector.Normalize();

		// Calculate the target angle
		double DotProduct = FVector::DotProduct(TurretForwardVector, TurretToLookDir);

		// DO NOT CHANGE TOLERANCE (0.008 also works ig. idk which value is better)
		constexpr double Tolerance = 0.008; // setting it to 0.01 fixed it now somehow when it wasnt working before. DO NOT CHANGE
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
		const double MaxDeltaAngle = MaxTurretRotationSpeed * DeltaTime;
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

	FVector GunLocation = BackCameraComp->GetComponentLocation() + (BackCameraComp->GetForwardVector() * 15000.0);

	FHitResult OutHit;
	auto bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		BackCameraComp->GetComponentLocation(),
		GunLocation,
		{ObjectTypeQuery1, ObjectTypeQuery6}, // should be worldstatic and destructible
		false,
		{this},
		bShowDebugTracesForTurret ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		OutHit,
		true
	);

	TurretImpactPoint = bHit ? OutHit.ImpactPoint : GunLocation;

	GunRotation = UKismetMathLibrary::FindLookAtRotation(
		GetMesh()->GetSocketLocation("gun_jnt"),
		TurretImpactPoint
	);

	DesiredGunElevation = GunRotation.Pitch;
	
	GunElevation = FMath::Clamp(
		FMath::FInterpTo(GunElevation, DesiredGunElevation, DeltaTime, GunElevationInterpSpeed),
		MinGunElevation,
		MaxGunElevation
	);

	GunElevation = FMath::Clamp(GunElevation, MinGunElevation, MaxGunElevation);

	SetGunElevation(GunElevation);

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) OutHit.ImpactPoint: %s"), *OutHit.ImpactPoint.ToString()),
	// 								  true, true, FLinearColor::Yellow, 0);
	//
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) GunLocation: %s"), *GunLocation.ToString()),
	// 								  true, true, FLinearColor::Yellow, 0);
	//
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) TurretImpactPoint: %s"), *TurretImpactPoint.ToString()),
	// 								  true, true, FLinearColor::Yellow, 0);
	//
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) DesiredGunElevation: %f"), DesiredGunElevation),
									  true, true, FLinearColor::Yellow, 0);
	//
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateGunElevation) GunElevation: %f"), GunElevation),
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
	
	// DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 16, FColor::White, false, 5.0f);

	if (bHit)
	{
		for (auto OutHit : OutHits)
		{
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
					RIF_Constant,
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
}

void ATankCharacter::OnHealthChanged_Implementation(float NewHealth, bool bIsRegenerating)
{
}

void ATankCharacter::OnShoot_Implementation()
{
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::OnShoot_Implementation)")),
	// 		true, true, FLinearColor::Yellow, 0);

	// Spawning muzzle fire and dust around the tank 
	SR_SpawnShootEmitters();

	// check if trace hit something. the trace is running on tick in another function
	if (TurretTraceHit.IsValidBlockingHit())
	{
		SpawnHitParticleSystem(TurretTraceHit);
		// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::OnShoot_Implementation) trace hit something")),
		// 		true, true, FLinearColor::Black, 5);
		SR_ApplyRadialDamage(TurretTraceHit);
	}
	else
	{
		// spawn projectile for the rest of the way.
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

	FVector TurretDirection = GetMesh()->GetSocketQuaternion("GunShootSocket").GetRightVector();
	TurretDirection.Normalize();
	FVector AngularImpulse = TurretDirection * -FMath::Abs(OnShootImpulseStrength); // Adjust multiplier for desired strength
	
	GetMesh()->AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);
}

void ATankCharacter::UpdateCameraPitchLimits_Implementation() const
{
	if (!PlayerController)
		return;

	// Adjust the pitch limits based on the tank's current pitch
	// double AdjustedPitchMin = BasePitchMin + GetActorRotation().Pitch;
	// double AdjustedPitchMax = BasePitchMax + GetActorRotation().Pitch;

	// AdjustedPitchMin = FMath::Fmod(AdjustedPitchMin + 180.0, 360.0) - 180.0;
	// AdjustedPitchMax = FMath::Fmod(AdjustedPitchMax + 180.0, 360.0) - 180.0;

	// Apply the adjusted limits
	PlayerController->PlayerCameraManager->ViewPitchMin = BasePitchMin;
	PlayerController->PlayerCameraManager->ViewPitchMax = BasePitchMax;

	
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateCameraPitchLimits) AdjustedPitchMin: %.5f"), AdjustedPitchMin),
	// 		true, true, FLinearColor::Yellow, 0);
	//
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateCameraPitchLimits) AdjustedPitchMax: %.5f"), AdjustedPitchMax),
	// 		true, true, FLinearColor::Yellow, 0);
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
	if (IsAimingIn() && FrontCameraComp)
		return FrontCameraComp;

	if (BackCameraComp)
		return BackCameraComp;
	
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
