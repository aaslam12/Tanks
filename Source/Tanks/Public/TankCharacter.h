// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TankInterface.h"
#include "WheeledVehiclePawn.h"
#include "TankCharacter.generated.h"

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
class TANKS_API ATankCharacter : public AWheeledVehiclePawn, public ITankInterface
{
	GENERATED_BODY()

	// UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	// TObjectPtr<UTankHealthComponent> HealthComponent;

	void InitializeHealthComponent();
	ATankCharacter();
	virtual ~ATankCharacter() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual auto GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const -> void override;

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

protected:
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
	virtual void OutlineTank_Implementation(const bool bActivate) override;
	// ITankInterface functions end
	
	/** Creates two box traces that combine to create a "+" sign attached to the gun turret. */
	UFUNCTION(BlueprintNativeEvent)
	void HighlightEnemyTanksIfDetected();

	/** Updates how much up or down you can look based on the tank rotation */
	UFUNCTION(BlueprintNativeEvent)
	void UpdateCameraPitchLimitsTick() const;

	/** All of these particle systems will be activated when the tank shoots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TArray<TObjectPtr<UParticleSystem>> ShootEmitterSystems;

	/** All of these particle systems will be activated when the tank shoots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TObjectPtr<UParticleSystem> ShootHitParticleSystem;
	
	/** The anim class to use for the tank. This is mainly here to set the anim bp in C++ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankAnimInstance> TankAnimInstanceClass;

	/** The health component class to use for the tank. Allows for blueprints to
	 * be added to the actor instead of pure C++ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankHealthComponent> TankHealthComponentClass;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomIn;

	// This is the maximum spring arm length when zooming out.
	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomOut;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=-40, UIMax=0))
	double BasePitchMin;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=30))
	double BasePitchMax;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=-10, UIMax=10, MakeStructureDefaultValue=0))
	double AbsoluteMinGunElevation;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=-10, UIMax=10, MakeStructureDefaultValue=0))
	double AbsoluteMaxGunElevation;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup", meta=(UIMin=0, UIMax=180, MakeStructureDefaultValue=90))
	double MaxTurretRotationSpeed;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double GunElevationInterpSpeed;

	// This is the minimum spring arm length when zooming in.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Setup")
	double MinGunElevation;

	// flat max gun elevation value
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	double MaxGunElevation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", Replicated)
	double CurrentTurretAngle;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTankAnimInstance> AnimInstance;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	/** The Player controller of this pawn */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Default")
	TObjectPtr<ATankController> PlayerController;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Default")
	double GunElevation;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Default")
	bool bIsInAir;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	double DesiredGunElevation;

private:
	// should only be used in the ATankCharacter::FindEnemyTanks function.
	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> HighlightedEnemyTanks;
	
	// should only be used in the ATankCharacter::FindEnemyTanks function.
	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> CurrentHitResults;

	// should only be used in the ATankCharacter::FindEnemyTanks function.
	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> VerticalHits;

	// should only be used in the ATankCharacter::FindEnemyTanks function.
	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> HorizontalHits;

protected:
	/** The Z offset of the "+" trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces", DisplayName="Box Trace Z Offset")
	double BoxTraceZOffset;

	/** How far to check and highlight enemy tanks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double BoxTraceLength;
	
	/** The vertical component of the "+" trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector VerticalLineTraceHalfSize; // FVector(10,10,300)

	/** The horizontal component of the "+" trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector HorizontalLineTraceHalfSize; // FVector(10,300,10)
	
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
	UPROPERTY(BlueprintReadOnly)
	FVector2D GunTraceScreenPosition;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly)
	FVector GunTraceEndpoint;
	
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
	/// C++ Getters
public:
	double GetMaxZoomIn() const { return MaxZoomIn; }
	double GetMaxZoomOut() const { return MaxZoomOut; }
	USceneComponent* GetShootSocket() const { return ShootSocket; }
	UCameraComponent* GetFrontCameraComp() const { return FrontCameraComp; }
	UCameraComponent* GetBackCameraComp() const { return BackCameraComp; }
	USpringArmComponent* GetFrontSpringArmComp() const { return FrontSpringArmComp; }
	USpringArmComponent* GetBackSpringArmComp() const { return BackSpringArmComp; }
	TArray<TObjectPtr<UParticleSystem>> GetShootEmitterSystems() const { return ShootEmitterSystems; }
	TObjectPtr<UParticleSystem> GetShootHitParticleSystem() const { return ShootHitParticleSystem; }
	bool IsInAir() const { return bIsInAir; }
	const TArray<FHitResult>& GetHighlightedEnemyTanks() const { return HighlightedEnemyTanks; }
	double GetAbsoluteMinGunElevation() const { return AbsoluteMinGunElevation; }
	double GetAbsoluteMaxGunElevation() const { return AbsoluteMaxGunElevation; }
	void SetMinGunElevation(double NewMinGunElevation) { MinGunElevation = NewMinGunElevation; }
	void SetMaxGunElevation(double NewMaxGunElevation) { MaxGunElevation = NewMaxGunElevation; }

	//////////////////////////////////////////////////////////////////
	/// Blueprint-Only Functions

	/* Returns the location of where the trace ends */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Tick")
	void GunSightTick(UPARAM(ref) FVector& EndPoint, UPARAM(ref) FVector2D& ScreenPosition);
};
