// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TankInterface.h"
#include "WheeledVehiclePawn.h"
#include "TankCharacter.generated.h"

class ATankController;
class UTankAnimInstance;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(Blueprintable)
class TANKS_API ATankCharacter : public AWheeledVehiclePawn, public ITankInterface
{
	GENERATED_BODY()



public:
	// Sets default values for this actor's properties
	ATankCharacter();
	virtual ~ATankCharacter() override;

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void TurretTurningTick(float DeltaTime);
	void CheckIfGunCanLowerElevationTick(float DeltaTime);
	void GunElevationTick(float DeltaTime);
	void IsInAirTick();
	virtual void HighlightTank_Implementation(const bool bActivate) override;
	void FindEnemyTanks(const FVector2D& GunTraceScreenPosition);
	bool IsEnemyNearTankCrosshair(const FVector& EnemyTankLocation, const FVector2D& CrosshairScreenPosition);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TArray<TObjectPtr<UParticleSystem>> ShootEmitterSystems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TObjectPtr<UParticleSystem> ShootHitParticleSystem;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankAnimInstance> TankAnimInstanceClass;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomIn;

	// This is the maximum spring arm length when zooming out.
	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomOut;

	// This is the minimum spring arm length when zooming in.
	// will take the absolute value of this 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=-10, UIMax=10, MakeStructureDefaultValue=0))
	double MinGunElevation;

	// flat max gun elevation value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=30, MakeStructureDefaultValue=20))
	double MaxGunElevation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=30, MakeStructureDefaultValue=20))
	double CurrentMinGunElevation;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup", meta=(UIMin=0, UIMax=180, MakeStructureDefaultValue=90))
	double MaxTurretRotationSpeed;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup", meta=(UIMin=2, UIMax=20, MakeStructureDefaultValue=10))
	double GunElevationInterpSpeed;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTankAnimInstance> AnimInstance;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Default")
	TObjectPtr<ATankController> PlayerController;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FVector GunLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	double GunElevation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	bool bIsInAir;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Default")
	FVector LastFreeLocation;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Default")
	double LastFreeGunElevation;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Default")
	double DesiredGunElevation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double LineTraceOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector VerticalLineTraceOffset; // 300

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double VerticalLineTraceSpacingOffset; // 5.3
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector HorizontalLineTraceOffset; // 165
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double HorizontalLineTraceSpacingMultiplier; // 3

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
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;

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

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetTurretRotation(double NewTurretAngle) const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetSkinType(double NewSkinType) const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetLightsEmissivity(double LightsEmissivity) const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetSpeed(double Speed);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetHatchesAngles(double HatchAngle);

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

	//////////////////////////////////////////////////////////////////
	/// Functions

	/* Returns the location of where the trace ends */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Tick")
	void GunSightTick(UPARAM(ref) FVector& EndPoint, UPARAM(ref) FVector2D& ScreenPosition);
};
