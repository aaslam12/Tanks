// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TankInterface.h"
// #include "WheeledVehiclePawn.h"
#include "GameFramework/TankGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Projectiles/ShootingInterface.h"
#include "Tanks/Template/MyProjectSportsCar.h"
#include "TankCharacter.generated.h"

class UWB_HealthBar;
class UWB_GunSight;

// forward declaring the enum
namespace EDrawDebugTrace
{
	enum Type : int;
}

class UTankTargetingSystem;
class UTankPowerUpManagerComponent;
class UTankAimAssistComponent;
class UNiagaraSystem;
class UWB_PlayerInfo;
class ATankPlayerState;
class UPostProcessComponent;
class UTankHighlightingComponent;
class ATankProjectile;
class URadialForceComponent;
class UTankHealthComponent;
class ATankController;
class UTankAnimInstance;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

FORCEINLINE uint32 GetTypeHash(const FHitResult& Hit)
{
	return FCrc::StrCrc32(*Hit.GetActor()->GetName());
}

FORCEINLINE bool operator==(const FHitResult& A, const FHitResult& B)
{
	return A.GetActor() == B.GetActor();
}

USTRUCT(BlueprintType)
struct FConeTraceConfig
{
	GENERATED_BODY()

	// Only one config should have this on !!!
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName="Is Used For Tank Targeting?", Category = "Setup|Cone Trace", meta=(SliderExponent=1.3))
	bool bIsUsedForTankTargeting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace", meta=(ClampMin=1, SliderExponent=1.5))
	int32 Steps;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace", meta=(UIMin=0.01, UIMax=2, ClampMin=0.01, SliderExponent=0.1))
	float CenterExponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace", meta=(SliderExponent=1.3))
	float StartRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace", meta=(UIMin=0.01, UIMax=1500, ClampMin=0.01, SliderExponent=0.1))
	float EndRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace", meta=(UIMin=0.01, UIMax=3, ClampMin=0.01, SliderExponent=0.1))
	float ConeLength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace")
	FLinearColor ConeTraceColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace")
	FLinearColor ConeTraceHitColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugTrace;

	FConeTraceConfig() : bIsUsedForTankTargeting(false), Steps(5), CenterExponent(1.258234), StartRadius(45),
	                     EndRadius(500), ConeLength(1500), ConeTraceColor(FLinearColor::Black),
	                     ConeTraceHitColor(FColor::Emerald), DrawDebugTrace(EDrawDebugTrace::ForOneFrame)
	{
	}
};

UCLASS(Abstract)
class TANKS_API ATankCharacter : public AMyProjectSportsCar, public ITankInterface, public IShootingInterface
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankHealthComponent> HealthComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankHighlightingComponent> TankHighlightingComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankPowerUpManagerComponent> TankPowerUpManagerComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankAimAssistComponent> TankAimAssistComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankTargetingSystem> TankTargetingSystem;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<URadialForceComponent> RadialForceComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> DamagedStaticMesh;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPostProcessComponent> TankPostProcessVolume;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWB_GunSight> GunSightWidget;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWB_HealthBar> HealthBarWidget;

	/**
	 *  Set in BP.
	 */
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWB_PlayerInfo> PlayerNameWidget;
	ETraceTypeQuery VisibilityTraceType;

	ATankCharacter();
	virtual ~ATankCharacter() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual auto GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const -> void override;
	virtual void PossessedBy(AController* NewController) override;

	void BindDelegates();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void SetWheelIndices();
	virtual void Tick(float DeltaTime) override;


	bool IsEnemy(AActor* OtherActor) const;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ResetCameraRotation();
	
	UFUNCTION(BlueprintNativeEvent)
	void HandleTakeDamage(float DamageAmount, class AController* EventInstigator, AActor* DamageCauser);
	
	UFUNCTION(BlueprintCallable, Category="Setup") 
	void SetComponentReferences(UTankHealthComponent* Health = nullptr) { HealthComponent = Health; }
	
	/** Sets all the necessary references to other classes */
	UFUNCTION(BlueprintNativeEvent)
	void SetDefaults();

	/** Traces spheres increasing in radius in the shape of a cone. */
	UFUNCTION(BlueprintNativeEvent)
	void ConeTraceTick();
	bool bConeTraceDisabled;

	// only used for cone trace.
	// is a member variable to improve performance from constant adding and
	// removing every frame.
	UPROPERTY()
	TArray<AActor*> AllHits;
	UPROPERTY()
	TArray<FHitResult> Hits;

	/** Traces from the muzzle to the point where it is looking at ahead. */
	UFUNCTION(BlueprintNativeEvent)
	void TurretTraceTick();
	
	/** Updates turret angle to where the camera is looking */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateTurretTurning(float DeltaTime);

	void UpdateDesiredTurretAngle();

	UFUNCTION(BlueprintCallable, Client, Unreliable)
	void CL_UpdateGunSightPosition();
public:
	UFUNCTION(BlueprintCallable)
	void SetDesiredTurretAngle(float TurretAngle);
private:
	double DesiredTurretAngle_C;
protected:
	UPROPERTY(BlueprintReadOnly)
	AActor* LockedTarget;

	/** Checks on tick if the gun can lower elevation. */
	UFUNCTION(BlueprintNativeEvent)
	void CheckIfGunCanLowerElevationTick(float DeltaTime);

	/** Updates the gun elevation to where the camera is looking */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateGunElevation(float DeltaTime);

	/** Line traces from the bottom of the tank to the floor to check if the tank is in the air. */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateIsInAir();

	// ITankInterface functions start
	virtual void OutlineTank_Implementation(const bool bActivate, const bool bIsFriend) override;
	virtual ETeam GetCurrentTeam_Implementation() override;
	virtual void PowerUpActivated_Implementation(const EPowerUpType PowerUpType) override;
	// ITankInterface functions end
	
	// IShootingInterface functions start
	virtual void ProjectileHit_Implementation(ATankProjectile* TankProjectile, UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	// IShootingInterface functions end

	UFUNCTION(BlueprintNativeEvent)
	void ApplyRadialDamage(const FHitResult& Hit);

	// Traces a sphere at hit point. Then applies radial impulse to any hit actors.
	// Scales with distance from the hit point exponentially.
	UFUNCTION(BlueprintNativeEvent)
	void ApplyRadialImpulseToObjects(const FHitResult& Hit);

	UFUNCTION(BlueprintNativeEvent)
	void ApplyTankShootImpulse() const;
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_ApplyRadialDamage(const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void MC_ApplyRadialDamage(const FHitResult& Hit);
	
	/** Updates how much up or down you can look based on the tank rotation */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateCameraPitchLimits() const;

	/** This function is bound to the controller's input shoot function */
	UFUNCTION(BlueprintNativeEvent)
	void OnShoot();

	UFUNCTION(BlueprintNativeEvent)
	void OnDie(APlayerState* AffectedPlayerState, bool bSelfDestruct, bool bShouldRespawn);

	UFUNCTION(BlueprintNativeEvent)
	void OnHealthChanged(float NewHealth, bool bIsRegenerating);
	
	// called when player respawns
	virtual void Restart() override;
	void Restart__Internal();
	
	/**  */
	UFUNCTION(Server, Reliable)
	void SR_Restart();

	/**  */
	UFUNCTION(NetMulticast, Reliable)
	void MC_Restart();

	/** All of these particle systems will be activated when the tank shoots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TObjectPtr<UMaterialInstanceDynamic> OutlineMaterial;

	/** All of these particle systems will be activated when the tank shoots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TArray<UParticleSystem*> ShootEmitterSystems;

	/** All of these particle systems will be activated when the tank shoots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TObjectPtr<UNiagaraSystem> ShootHitParticleSystem;
	
	/** The anim class to use for the tank. This is mainly here to set the anim bp in C++ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankAnimInstance> TankAnimInstanceClass;

	/** The health component class to use for the tank. Allows for blueprints to
	 * be added to the actor instead of pure C++ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankHealthComponent> TankHealthComponentClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TObjectPtr<UStaticMesh> OnDieStaticMesh;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Camera", meta=(UIMin=0, UIMax=5000))
	double MaxZoomIn;

	// This is the maximum spring arm length when zooming out.
	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Camera", meta=(UIMin=0, UIMax=5000))
	double MaxZoomOut;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Camera", meta=(UIMin=-40, UIMax=0))
	double BasePitchMin;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Camera", meta=(UIMin=0, UIMax=30))
	double BasePitchMax;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Gun Elevation", meta=(UIMin=-10, UIMax=10, MakeStructureDefaultValue=0))
	double AbsoluteMinGunElevation;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Gun Elevation", meta=(UIMin=-10, UIMax=10, MakeStructureDefaultValue=0))
	double AbsoluteMaxGunElevation;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Turret", meta=(UIMin=0.01, UIMax=180, ClampMin=0.01, MakeStructureDefaultValue=90, SliderExponent=2))
	double TurretRotationSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Turret", meta=(UIMin=0.01, UIMax=180, ClampMin=0.01, MakeStructureDefaultValue=90, SliderExponent=2))
	double AimingTurretRotationSpeed;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Turret", meta=(UIMin=0.01, UIMax=50, ClampMin=0.01, MakeStructureDefaultValue=30, SliderExponent=2))
	double MaxTurretElevationAdjustSpeed;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Turret", meta=(UIMin=0.01, UIMax=150, ClampMin=0.01, MakeStructureDefaultValue=30, SliderExponent=2))
	double OnShootImpulseStrength;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Gun Elevation", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double GunElevationInterpSpeed;

public:
	TArray<int32> LeftWheelIndices;
	TArray<int32> RightWheelIndices;

protected:
	/**
	 * The base damage to apply, i.e. the damage at the origin.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Damage", meta=(UIMin=100, UIMax=2000, MakeStructureDefaultValue=10))
	double BaseDamage; // 1000

	/**
	 * Radius of the full damage area, from Origin
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Damage", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double DamageInnerRadius;

	/**
	 * Radius of the minimum damage area, from Origin
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Damage", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double DamageOuterRadius;

	/**
	 * Falloff exponent of damage from DamageInnerRadius to DamageOuterRadius
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Gameplay|Damage", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double DamageFalloffExponent;

	// Toggles all debug traces for turret. Is controlled in BP or in game.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Debug")
	bool bShowDebugTracesForTurret;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Cone Trace", meta=(SliderExponent=1.3))
	TArray<FConeTraceConfig> ConeTraceConfigs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Radial Impulse at hit location", meta=(UIMin=0.1, UIMax=3, SliderExponent=0.1))
	float ImpulseStrengthExponent;

	// This is the minimum spring arm length when zooming in.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Setup|Gameplay|Gun Elevation")
	double MinGunElevation;

	// flat max gun elevation value
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	double MaxGunElevation;
	
	/** Turret Up/Down Rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Default")
	double GunElevation;

	/** Turret Left/Right Rotation */
	UPROPERTY(BlueprintReadOnly, Category = "Default", Replicated)
	double CurrentTurretAngle;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<UTankAnimInstance> AnimInstance;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<ATankPlayerState> TankPlayerState;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	/** The Player controller of this pawn */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<ATankController> PlayerController;

	/** The camera trace hit location. if there is no hit, this will be the same as the 'ActiveCameraEnd' */
	UPROPERTY(BlueprintReadWrite, Category = "Default")
	FVector DesiredCameraImpactPoint;

	/** The interpolated camera impact point. see 'ATankCharacter::UpdateGunElevation' for more information */
	UPROPERTY(BlueprintReadWrite, Category = "Default")
	FVector CameraImpactPoint;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UCameraComponent> PrevCameraComp;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FRotator DesiredTurretTurn;
	
	/** The location of the turret muzzle */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FVector TurretStart;

	/** The location of the end of the turret muzzle forward trace. (does not change if there are hits) */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FVector TurretEnd;

	/** The Hit Result of the turret muzzle forward trace */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHitResult TurretTraceHit;

	/** The location of the active camera trace start */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FVector ActiveCameraStart;

	/** The location of the active camera trace end (does not change if there are hits) */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FVector ActiveCameraEnd;

	/** The Hit Result of the camera forward trace */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHitResult CameraTraceHit;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bIsInAir;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, Category = "Default")
	double DesiredGunElevation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, Category = "Default")
	FRotator GunRotation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default", Replicated)
	ETeam CurrentTeam;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<UTankGameInstance> GameInstance;
	
public:
	/** Set by Player State when the game starts */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", Replicated)
	FString PlayerName;

protected:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<USceneComponent> ShootSocket;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UCameraComponent> FrontCameraComp;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UCameraComponent> MiddleCameraComp;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UCameraComponent> BackCameraComp;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<USpringArmComponent> BackSpringArmComp;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<USpringArmComponent> MiddleSpringArmComp;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<USpringArmComponent> FrontSpringArmComp;

public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;

	/** Is the player in first person mode? */
	UPROPERTY(BlueprintReadWrite)
	bool bAimingIn;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> TracksMaterial;

public:
	void SpawnProjectileFromPool();
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetGunElevation(double NewGunElevation) const;

	/** Simple function that spawns a new particle everytime. */
	UFUNCTION(BlueprintCallable)
	void SpawnHitParticleSystem(const FHitResult& Location) const;

protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_SetGunElevation(double NewGunElevation) const;
	
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetGunElevation(double NewGunElevation) const;
public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetTurretRotation(double NewTurretAngle) const;
protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_SetTurretRotation(double NewTurretAngle) const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetTurretRotation(double NewTurretAngle) const;
public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetSkinType(double NewSkinType) const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetLightsEmissivity(double LightsEmissivity) const;
protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_SetLightsEmissivity(double LightsEmissivity) const;
	
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetLightsEmissivity(double LightsEmissivity) const;
public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetSpeed(double Speed);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_SetSpeed(double Speed);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetSpeed(double Speed);
public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetHatchesAngles(double HatchAngle) const;
protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetHatchesAngles(double HatchAngle);

	UFUNCTION(BlueprintCallable)
	void SpawnShootEmitters() const;

	UFUNCTION(BlueprintCallable, Server, Unreliable)
	void SR_SpawnShootEmitters();
	
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void MC_SpawnShootEmitters();

public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="IsAimingIn")
	bool IsAimingIn() const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const UCameraComponent* GetActiveCamera() const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const ATankController* GetPlayerController() const;
	
protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent)
	USceneComponent* GetShootSocketFromBP() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	UCameraComponent* GetFrontCameraFromBP() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	UCameraComponent* GetBackCameraFromBP() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	USpringArmComponent* GetBackSpringArmFromBP() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	USpringArmComponent* GetFrontSpringArmFromBP() const;
public:
	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	void ToggleMiddleCamera();
protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, DisplayName="SetWheelSmokeIntensity")
	void SetWheelSmoke(float Intensity);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, DisplayName="MC_SetWheelSmokeIntensity", NetMulticast, Reliable)
	void MC_SetWheelSmoke(float Intensity);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetMaxZoomIn() const { return MaxZoomIn; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetMaxZoomOut() const { return MaxZoomOut; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE USceneComponent* GetShootSocket() const { return ShootSocket; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UCameraComponent* GetFrontCameraComp() const { return FrontCameraComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UCameraComponent* GetMiddleCameraComp() const { return MiddleCameraComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UCameraComponent* GetBackCameraComp() const { return BackCameraComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE USpringArmComponent* GetFrontSpringArmComp() const { return FrontSpringArmComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE USpringArmComponent* GetMiddleSpringArmComp() const { return MiddleSpringArmComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE USpringArmComponent* GetBackSpringArmComp() const { return BackSpringArmComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE TArray<UParticleSystem*> GetShootEmitterSystems() const { return ShootEmitterSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UNiagaraSystem* GetShootHitParticleSystem() const { return ShootHitParticleSystem; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsInAir() const { return bIsInAir; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetAbsoluteMinGunElevation() const { return AbsoluteMinGunElevation; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetAbsoluteMaxGunElevation() const { return AbsoluteMaxGunElevation; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMinGunElevation(double NewMinGunElevation) { MinGunElevation = NewMinGunElevation; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMaxGunElevation(double NewMaxGunElevation) { MaxGunElevation = NewMaxGunElevation; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UPostProcessComponent* GetTankPostProcessVolume() const { return TankPostProcessVolume; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UTankHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UTankHighlightingComponent* GetTankHighlightingComponent() const { return TankHighlightingComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE URadialForceComponent* GetRadialForceComponent() const { return RadialForceComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetAbsoluteMinGunElevation1() const { return AbsoluteMinGunElevation; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetAbsoluteMaxGunElevation1() const { return AbsoluteMaxGunElevation; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetMaxTurretRotationSpeed() const { return TurretRotationSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetBaseDamage() const { return BaseDamage; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetGunElevation() const { return GunElevation; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetCurrentTurretAngle() const { return CurrentTurretAngle; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE double GetGunElevationInterpSpeed() const { return GunElevationInterpSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UTankAnimInstance* GetAnimInstance() const { return AnimInstance; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool CanLockOn() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCanLockOn(bool bCond) const;
};
