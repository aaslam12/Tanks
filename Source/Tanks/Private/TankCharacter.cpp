// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Public/TankCharacter.h"

#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "TankController.h"
#include "TanksGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/TankHealthComponent.h"
#include "Components/TankHighlightingComponent.h"
#include "GameFramework/TankGameInstance.h"
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
								  RadialForceComponent(CreateDefaultSubobject<URadialForceComponent>("RadialForceComponent")),
								  DamagedStaticMesh(CreateDefaultSubobject<UStaticMeshComponent>("Damaged Tank Mesh")),
								  MaxZoomIn(500), MaxZoomOut(2500), BasePitchMin(-20.0), BasePitchMax(10.0),
                                  AbsoluteMinGunElevation(-5), AbsoluteMaxGunElevation(30), MaxTurretRotationSpeed(90),
                                  GunElevationInterpSpeed(10), BaseDamage(500),
                                  MinGunElevation(-15), MaxGunElevation(20), CurrentTurretAngle(0), GunElevation(0),
                                  bIsInAir(false), 
                                  DesiredGunElevation(0), 
                                  LookValues(), MoveValues(), bAimingIn(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RadialForceComponent->SetupAttachment(RootComponent);
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
}

void ATankCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentTurretAngle);
	DOREPLIFETIME(ThisClass, CurrentTeam);
}

void ATankCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// cache the EnhancedInputComponent to bind controls later in BeginPlay after the controller is available.
	if (PlayerInputComponent)
		if (Cast<UEnhancedInputComponent>(PlayerInputComponent))
			EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
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

	if (UWorld* World = GetWorld())
		World->Exec(World, TEXT("p.Vehicle.SetMaxMPH 30"));

	GameInstance = Cast<UTankGameInstance>(GetGameInstance());
	PlayerName = GameInstance->GetPlayerName();
}

void ATankCharacter::BindDelegates()
{
	if (PlayerController)
	{
		PlayerController->OnShoot.AddDynamic(this, &ATankCharacter::OnShoot);
	}

	if (HealthComponent)
	{
		HealthComponent->OnDie.AddDynamic(this, &ATankCharacter::OnDie);
	}
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

		// Execute other logic
		UpdateTurretTurning(DeltaTime);
		UpdateGunElevation(DeltaTime);
		CheckIfGunCanLowerElevationTick(DeltaTime);
		UpdateCameraPitchLimits();

		UpdateIsInAir();

		if (GetPlayerState())
			if (Cast<ATankPlayerState>(GetPlayerState()))
				TankPlayerState = Cast<ATankPlayerState>(GetPlayerState());

		// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATanksCharacter::Tick) Tick running")),
		// 	true, true, FLinearColor::Yellow, 0);
	}
	
	auto e = UKismetMathLibrary::FindLookAtRotation(
		GetMesh()->GetSocketLocation("gun_jnt"),
		GetActorLocation() + FVector(0, 0, 200)
	).Vector() * RadialForceComponent->ImpulseStrength;

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::Tick) FindLookAtRotation: %s"), *e.ToString()),
			true, true, FLinearColor::Yellow, 0);

	
}

float ATankCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (Execute_GetCurrentTeam(this).Equals(Execute_GetCurrentTeam(DamageCauser)) == false)
		HealthComponent->OnTakeDamaged(this, DamageAmount, nullptr, EventInstigator, DamageCauser);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ATankCharacter::UpdateTurretTurning_Implementation(float DeltaTime)
{
	if (!Controller || !GetMesh() || !AnimInstance)
		return;

	if (bAimingIn)
	{
		// first person turret rotation
		float DesiredTurretAngle = AnimInstance->TurretAngle + FMath::Clamp(LookValues.X * 25, -MaxTurretRotationSpeed / 2, MaxTurretRotationSpeed / 2);

		// Smoothly interpolate towards the desired angle
		CurrentTurretAngle = FMath::FInterpTo(
			CurrentTurretAngle,
			DesiredTurretAngle,
			DeltaTime,
			3.0f
		);

		// Apply the interpolated value to the turret rotation
		SetTurretRotation(CurrentTurretAngle);
	}
	else
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

		// DO NOT CHANGE TOLERANCE
		constexpr double Tolerance = 0.01; // setting it to 0.01 fixed it now somehow when it wasnt working before. DO NOT CHANGE
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
	}
}

void ATankCharacter::CheckIfGunCanLowerElevationTick_Implementation(float DeltaTime)
{
	if (!PlayerController)
		return;
	
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
		EDrawDebugTrace::ForOneFrame,
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
		EDrawDebugTrace::ForOneFrame,
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
				MinGunElevation = FMath::Clamp(MinGunElevation + 1, AbsoluteMinGunElevation, AbsoluteMaxGunElevation); // 17
				PlayerController->SetShootingBlocked(true);
				return;
			}

			if (bBottomDetectTank == true && bTopDetectTank == false)
			{
				MinGunElevation = GunElevation;
				PlayerController->SetShootingBlocked(false);
				return;
			}
			
		}
		else // 3rd person
		{
			if (bTopDetectTank == true) // if tank body physical material is detected and is the same actor
			{
				MinGunElevation = FMath::Clamp(MinGunElevation + 1, AbsoluteMinGunElevation, AbsoluteMaxGunElevation); // 17
				PlayerController->SetShootingBlocked(true);
			}

			if (bBottomDetectTank == true && bTopDetectTank == false)
			{
				MinGunElevation = GunElevation;
				return;
			}
		}
		
	}
	else
	{
		// Update the last free gun elevation
		// if (bAimingIn)
		// 	MinGunElevation = FMath::Max(FMath::Min(GunElevation, DesiredGunElevation), AbsoluteMinGunElevation);
		MinGunElevation = FMath::Clamp(MinGunElevation - 0.2, AbsoluteMinGunElevation, GunElevation);

		PlayerController->SetShootingBlocked(false);
	}
}

void ATankCharacter::UpdateGunElevation_Implementation(float DeltaTime)
{
	if (!BackCameraComp)
		return;

	if (bAimingIn)
	{
		GunElevation += LookValues.Y * -1;
		GunElevation = FMath::Clamp(GunElevation, MinGunElevation, MaxGunElevation);
		SetGunElevation(GunElevation);
		return;
	}

	FVector GunLocation = BackCameraComp->GetComponentLocation() + (BackCameraComp->GetForwardVector() * 7000.0);

	FHitResult OutHit;
	auto bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(),
		BackCameraComp->GetComponentLocation(),
		GunLocation,
		{ObjectTypeQuery1, ObjectTypeQuery6}, // should be worldstatic and destructible
		false,
		{this},
		EDrawDebugTrace::None,
		OutHit,
		true
	);

	auto LookAtRot = UKismetMathLibrary::FindLookAtRotation(
		GetMesh()->GetSocketLocation("gun_jnt"),
		bHit ? OutHit.Location : GunLocation
	);

	GunRotation = LookAtRot;
	DesiredGunElevation = GunRotation.Pitch;
	
	GunElevation = FMath::Clamp(
		FMath::FInterpTo(GunElevation, DesiredGunElevation, DeltaTime, GunElevationInterpSpeed),
		MinGunElevation,
		MaxGunElevation
	);

	GunElevation = FMath::Clamp(GunElevation, MinGunElevation, MaxGunElevation);

	SetGunElevation(GunElevation);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateCameraPitchLimitsTick) GunElevation: %.5f"), GunElevation),
			true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateCameraPitchLimitsTick) GunRotation: %s"), *GunRotation.ToString()),
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
	// 	EDrawDebugTrace::None,
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
			if (GetMesh()->CustomDepthStencilValue == 0)
			{
				if (GetMesh()->CustomDepthStencilValue != FriendStencilValue)
				{
					GetMesh()->SetCustomDepthStencilValue(FriendStencilValue);
				}
			}
		}
		else
		{
			if (GetMesh()->CustomDepthStencilValue != FriendStencilValue)
			{
				if (GetMesh()->CustomDepthStencilValue != EnemyStencilValue)
				{
					GetMesh()->SetCustomDepthStencilValue(EnemyStencilValue);
				}
			}
		}
	}
	else
	{
		if (GetMesh()->CustomDepthStencilValue == EnemyStencilValue)
		{
			if (GetMesh()->CustomDepthStencilValue != 0)
			{
				GetMesh()->SetCustomDepthStencilValue(0);
			}
		}
	}
}

FString ATankCharacter::GetCurrentTeam_Implementation()
{
	if (TankPlayerState)
		return TankPlayerState->GetCurrentTeam();
	return TEXT(""); // empty string
}

void ATankCharacter::ProjectileHit_Implementation(ATankProjectile* TankProjectile, UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	IShootingInterface::ProjectileHit_Implementation(TankProjectile, HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	SR_ApplyRadialDamage(Hit);
	TankProjectile->ResetTransform();
}

void ATankCharacter::ApplyRadialDamage(const FHitResult& Hit)
{
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(),
		BaseDamage, BaseDamage * 0.1, Hit.Location,
		DamageInnerRadius, DamageOuterRadius, DamageFalloff, UTankDamageType::StaticClass(),
		{}, this, GetController());

	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::ApplyRadialDamage) %s Applying damage to: %s %.5f"), *GetName(), *Hit.GetActor()->GetName(), BaseDamage),
	// 		true, true, FLinearColor::Red, 15);
}

void ATankCharacter::SR_ApplyRadialDamage_Implementation(const FHitResult& Hit)
{
	MC_ApplyRadialDamage(Hit);
}

void ATankCharacter::MC_ApplyRadialDamage_Implementation(const FHitResult& Hit)
{
	ApplyRadialDamage(Hit);
}

void ATankCharacter::OnDie_Implementation()
{
	if (OnDieStaticMesh)
	{
		GetMesh()->SetHiddenInGame(true);
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		
		DamagedStaticMesh->SetWorldTransform(GetActorTransform()); // TODO. add a small impulse to the tank and bounce it up
		DamagedStaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		DamagedStaticMesh->SetHiddenInGame(false);
		DamagedStaticMesh->SetVisibility(true);

		if (PlayerController)
			PlayerController->OnDie();
	}
}

void ATankCharacter::OnShoot_Implementation()
{
	// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::OnShoot_Implementation)")),
	// 		true, true, FLinearColor::Yellow, 0);

	constexpr double ShootTraceDistance = 15000.0;

	// Spawning muzzle fire and dust around the tank 
	SR_SpawnShootEmitters();
	FVector TurretStart = GetMesh()->GetSocketLocation("GunShootSocket");
	FVector End = TurretStart + GetMesh()->GetSocketQuaternion("GunShootSocket").GetForwardVector() * ShootTraceDistance;

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
		SpawnHitParticleSystem(OutHit.Location);
		SR_ApplyRadialDamage(OutHit);
	}
	else
	{
		// spawn projectile for the rest of the way.
		auto GameMode = Cast<ATanksGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		
		if (GameMode != nullptr)
		{
			if (GameMode->ProjectilePool != nullptr)
			{
				auto Rot = UKismetMathLibrary::FindLookAtRotation(OutHit.TraceStart, OutHit.TraceEnd);
				GameMode->ProjectilePool->SpawnFromPool(FTransform(Rot, OutHit.TraceEnd));
			}
		}
	}
}

void ATankCharacter::SR_Shoot_Implementation()
{
	
}

void ATankCharacter::UpdateCameraPitchLimits_Implementation() const
{
	if (!PlayerController)
		return;

	// Adjust the pitch limits based on the tank's current pitch
	double AdjustedPitchMin = BasePitchMin + GetActorRotation().Pitch;
	double AdjustedPitchMax = BasePitchMax + GetActorRotation().Pitch;

	AdjustedPitchMin = FMath::Fmod(AdjustedPitchMin + 180.0, 360.0) - 180.0;
	AdjustedPitchMax = FMath::Fmod(AdjustedPitchMax + 180.0, 360.0) - 180.0;

	// Apply the adjusted limits
	PlayerController->PlayerCameraManager->ViewPitchMin = BasePitchMin;
	PlayerController->PlayerCameraManager->ViewPitchMax = BasePitchMax;

	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateCameraPitchLimits) AdjustedPitchMin: %.5f"), AdjustedPitchMin),
			true, true, FLinearColor::Yellow, 0);

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankCharacter::UpdateCameraPitchLimits) AdjustedPitchMax: %.5f"), AdjustedPitchMax),
			true, true, FLinearColor::Yellow, 0);
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

void ATankCharacter::SpawnHitParticleSystem(const FVector& Location) const
{
	UGameplayStatics::SpawnEmitterAtLocation( GetWorld(),
		ShootHitParticleSystem, Location,
		FRotator( 0), FVector(1), true,
		EPSCPoolMethod::AutoRelease
	);
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

	if (HasAuthority())
		BodyMaterial->SetScalarParameterValue("EmissiveMultiplier", LightsEmissivity);
	else
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

void ATankCharacter::SetHatchesAngles(double HatchAngle)
{
	if (AnimInstance)
		AnimInstance->HatchAngle = HatchAngle;
}

void ATankCharacter::SpawnShootEmitters()
{
	for (auto ParticleSystem : GetShootEmitterSystems())
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, GetShootSocket()->GetComponentTransform());

	RadialForceComponent->FireImpulse();
	
	GetMesh()->AddImpulseAtLocation(
		UKismetMathLibrary::FindLookAtRotation(
			GetMesh()->GetSocketLocation("gun_jnt"),
			GetActorLocation() + FVector(0, 0, 200)
		).Vector() * RadialForceComponent->ImpulseStrength * 10 + GetVehicleMovementComponent()->GetForwardSpeedMPH() * 1000,
		GetMesh()->GetSocketLocation("gun_jnt") 
	);
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
