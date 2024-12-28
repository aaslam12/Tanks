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
	// Sets default values for this actor's properties
	ATankCharacter();
	virtual ~ATankCharacter() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual auto GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const -> void override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void SetDefaults();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void TurretTurningTick(float DeltaTime);
	void CheckIfGunCanLowerElevationTick(float DeltaTime);
	void GunElevationTick(float DeltaTime);
	void IsInAirTick();
	virtual void OutlineTank_Implementation(const bool bActivate) override;
	// Creates two box traces that combine to create a "+" sign attached to the gun turret.
	void HighlightEnemyTanksIfDetected();
	void UpdateCameraPitchLimitsTick() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TArray<TObjectPtr<UParticleSystem>> ShootEmitterSystems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TObjectPtr<UParticleSystem> ShootHitParticleSystem;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankAnimInstance> TankAnimInstanceClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankHealthComponent> TankHealthComponentClass;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomIn;

	// This is the maximum spring arm length when zooming out.
	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomOut;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=-40, UIMax=0))
	double BasePitchMin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=30))
	double BasePitchMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=-10, UIMax=10, MakeStructureDefaultValue=0))
	double AbsoluteMinGunElevation;

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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", Replicated)
	double CurrentTurretAngle;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTankAnimInstance> AnimInstance;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Default")
	TObjectPtr<ATankController> PlayerController;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Default")
	double GunElevation;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Default")
	bool bIsInAir;
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double LineTraceOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double LineTraceForwardVectorMultiplier;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector VerticalLineTraceHalfSize; // FVector(10,10,300)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector HorizontalLineTraceHalfSize; // FVector(10,300,10)
	
	UPROPERTY()
	TObjectPtr<USceneComponent> ShootSocket;

	UPROPERTY()
	TObjectPtr<UCameraComponent> FrontCameraComp;
	
	UPROPERTY()
	TObjectPtr<UCameraComponent> BackCameraComp;

	UPROPERTY()
	TObjectPtr<USpringArmComponent> BackSpringArmComp;

	UPROPERTY()
	TObjectPtr<USpringArmComponent> FrontSpringArmComp;

public:
	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;

	UPROPERTY(BlueprintReadOnly)
	bool bAimingIn;

	UPROPERTY(BlueprintReadOnly)
	FVector2D GunTraceScreenPosition;

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
	/// Getters
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
	/// Functions

	/* Returns the location of where the trace ends */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Tick")
	void GunSightTick(UPARAM(ref) FVector& EndPoint, UPARAM(ref) FVector2D& ScreenPosition);
};
