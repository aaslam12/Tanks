// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TankInterface.h"
// #include "WheeledVehiclePawn.h"
#include "GameFramework/TankGameInstance.h"
#include "Projectiles/ShootingInterface.h"
#include "Tanks/Template/MyProjectSportsCar.h"
#include "TankCharacter.generated.h"

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

UCLASS(Abstract)
class TANKS_API ATankCharacter : public AMyProjectSportsCar, public ITankInterface, public IShootingInterface
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankHealthComponent> HealthComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankHighlightingComponent> TankHighlightingComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<URadialForceComponent> RadialForceComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> DamagedStaticMesh;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPostProcessComponent> TankPostProcessVolume;

	/**
	 *  Set in BP.
	 */
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWB_PlayerInfo> PlayerNameWidget;

	ATankCharacter();
	virtual ~ATankCharacter() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual auto GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const -> void override;
	virtual void PossessedBy(AController* NewController) override;

	void BindDelegates();
	
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void TurretTraceTick();
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ResetCameraRotation();
	
	UFUNCTION(BlueprintNativeEvent)
	void HandleTakeDamage(float DamageAmount, class AController* EventInstigator, AActor* DamageCauser);
	
	UFUNCTION(BlueprintCallable, Category="Setup") 
	void SetComponentReferences(UTankHealthComponent* Health) { HealthComponent = Health; }
	
	/** Sets all the needed references to other classes */
	UFUNCTION(BlueprintNativeEvent)
	void SetDefaults();

	/** Updates turret angle to where the camera is looking */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateTurretTurning(float DeltaTime);

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
	// ITankInterface functions end
	
	// IShootingInterface functions start
	virtual void ProjectileHit_Implementation(ATankProjectile* TankProjectile, UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	// IShootingInterface functions end

	UFUNCTION(BlueprintNativeEvent)
	void ApplyRadialDamage(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SR_ApplyRadialDamage(const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void MC_ApplyRadialDamage(const FHitResult& Hit);
	
	/** Updates how much up or down you can look based on the tank rotation */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateCameraPitchLimits() const;

	/** This function is binded to the controller's input shoot function */
	UFUNCTION(BlueprintNativeEvent)
	void OnShoot();

	UFUNCTION(BlueprintNativeEvent)
	void OnDie(APlayerState* AffectedPlayerState, bool bSelfDestruct, bool bShouldRespawn);
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Turret", meta=(UIMin=0, UIMax=180, MakeStructureDefaultValue=90))
	double MaxTurretRotationSpeed;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Gun Elevation", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double GunElevationInterpSpeed;

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

	// Toggles all debug traces for turret
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup|Gameplay|Damage")
	bool bShowDebugTracesForTurret;

	// This is the minimum spring arm length when zooming in.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Setup|Gameplay|Gun Elevation")
	double MinGunElevation;

	// flat max gun elevation value
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	double MaxGunElevation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default", Replicated)
	double CurrentTurretAngle;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FHitResult TurretTraceHit;

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

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	double GunElevation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FVector TurretImpactPoint;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FVector TurretEnd;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bIsInAir;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	double DesiredGunElevation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
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
	UPROPERTY()
	TObjectPtr<USceneComponent> ShootSocket;

	/** Please add a variable description */
	UPROPERTY()
	TObjectPtr<UCameraComponent> FrontCameraComp;
	
	/** Please add a variable description */
	UPROPERTY()
	TObjectPtr<UCameraComponent> BackCameraComp;

	/** Please add a variable description */
	UPROPERTY()
	TObjectPtr<USpringArmComponent> BackSpringArmComp;

	/** Please add a variable description */
	UPROPERTY()
	TObjectPtr<USpringArmComponent> FrontSpringArmComp;

public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;

	/** Is the player in first person mode? */
	UPROPERTY(BlueprintReadOnly)
	bool bAimingIn;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> TracksMaterial;

public:
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
	void SetHatchesAngles(double HatchAngle);
protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MC_SetHatchesAngles(double HatchAngle);

	UFUNCTION(BlueprintCallable)
	void SpawnShootEmitters();

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
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent, DisplayName="GetShootSocket")
	USceneComponent* GetShootSocke() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	UCameraComponent* GetFrontCamera() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	UCameraComponent* GetBackCamera() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	USpringArmComponent* GetBackSpringArm() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	USpringArmComponent* GetFrontSpringArm() const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, DisplayName="SetWheelSmokeIntensity")
	void SetWheelSmoke(float Intensity);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, DisplayName="MC_SetWheelSmokeIntensity", NetMulticast, Reliable)
	void MC_SetWheelSmoke(float Intensity);

	//////////////////////////////////////////////////////////////////
	/// Pure C++ Getters and Setters
public:
	FORCEINLINE double GetMaxZoomIn() const { return MaxZoomIn; }
	FORCEINLINE double GetMaxZoomOut() const { return MaxZoomOut; }
	FORCEINLINE USceneComponent* GetShootSocket() const { return ShootSocket; }
	FORCEINLINE UCameraComponent* GetFrontCameraComp() const { return FrontCameraComp; }
	FORCEINLINE UCameraComponent* GetBackCameraComp() const { return BackCameraComp; }
	FORCEINLINE USpringArmComponent* GetFrontSpringArmComp() const { return FrontSpringArmComp; }
	FORCEINLINE USpringArmComponent* GetBackSpringArmComp() const { return BackSpringArmComp; }
	FORCEINLINE TArray<UParticleSystem*> GetShootEmitterSystems() const { return ShootEmitterSystems; }
	FORCEINLINE UNiagaraSystem* GetShootHitParticleSystem() const { return ShootHitParticleSystem; }
	FORCEINLINE bool IsInAir() const { return bIsInAir; }
	FORCEINLINE double GetAbsoluteMinGunElevation() const { return AbsoluteMinGunElevation; }
	FORCEINLINE double GetAbsoluteMaxGunElevation() const { return AbsoluteMaxGunElevation; }
	FORCEINLINE void SetMinGunElevation(double NewMinGunElevation) { MinGunElevation = NewMinGunElevation; }
	FORCEINLINE void SetMaxGunElevation(double NewMaxGunElevation) { MaxGunElevation = NewMaxGunElevation; }
	FORCEINLINE UPostProcessComponent* GetTankPostProcessVolume() const { return TankPostProcessVolume; }
	FORCEINLINE UTankHealthComponent* GetHealthComponent() const { return HealthComponent; }
	FORCEINLINE UTankHighlightingComponent* GetTankHighlightingComponent() const { return TankHighlightingComponent; }
	FORCEINLINE URadialForceComponent* GetRadialForceComponent() const { return RadialForceComponent; }


	//////////////////////////////////////////////////////////////////
	/// Blueprint-Only Functions

	/* Returns the location of where the trace ends */
	// UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Tick")
	// void GunSightTick(UPARAM(ref) FVector& EndPoint, UPARAM(ref) FVector2D& ScreenPosition);
};
