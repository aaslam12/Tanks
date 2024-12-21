// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "TanksCharacter.generated.h"

class ATankController;
class UTankAnimInstance;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(Blueprintable)
class TANKS_API ATanksCharacter : public AWheeledVehiclePawn
{
	GENERATED_BODY()



public:
	// Sets default values for this actor's properties
	ATanksCharacter();
	virtual ~ATanksCharacter() override;

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setup")
	TArray<TObjectPtr<UParticleSystem>> ShootEmitterSystems;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankAnimInstance> TankAnimInstanceClass;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomIn;

	// Should be greater than MaxZoomIn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup", meta=(UIMin=0, UIMax=5000))
	double MaxZoomOut;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTankAnimInstance> AnimInstance;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Default")
	TObjectPtr<ATankController> PlayerController;

	UPROPERTY()
	TObjectPtr<USceneComponent> ShootSocket;

	UPROPERTY()
	TObjectPtr<UCameraComponent> FrontCameraComp;
	
	UPROPERTY()
	TObjectPtr<UCameraComponent> BackCameraComp;

	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArmComp;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;
	
	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> TracksMaterial;

public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetGunElevation(double GunElevation);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetTurretRotation(double TurretAngle);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetSkinType(double SkinType);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetLightsEmissivity(double LightsEmissivity);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetSpeed(double Speed);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetHatchesAngles(double HatchAngle);

protected:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent)
	USceneComponent* GetShootSocke() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	UCameraComponent* GetFrontCamera() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	UCameraComponent* GetBackCamera() const;

	/** Please add a function description */
	UFUNCTION(BlueprintImplementableEvent)
	USpringArmComponent* GetSpringArm() const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetWheelSmoke(float Intensity);

public:
	double GetMaxZoomIn() const { return MaxZoomIn; }

	double GetMaxZoomOut() const { return MaxZoomOut; }

	USceneComponent* GetShootSocket() const { return ShootSocket; }

	UCameraComponent* GetFrontCameraComp() const { return FrontCameraComp; }

	UCameraComponent* GetBackCameraComp() const { return BackCameraComp; }

	USpringArmComponent* GetSpringArmComp() const { return SpringArmComp; }

	TArray<TObjectPtr<UParticleSystem>> GetShootEmitterSystems() const { return ShootEmitterSystems; }
};
