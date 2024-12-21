// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "TanksCharacter.generated.h"

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

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TurnAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseWheelUpAction;
	
	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseWheelDownAction;

	/** Handbrake Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HandbrakeAction;

public:
	// Sets default values for this actor's properties
	ATanksCharacter();
	virtual ~ATanksCharacter() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	void BindControls();

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
	TObjectPtr<APlayerController> PlayerController;

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

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool StopTurn;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double VehicleYaw;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** Please add a variable description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInstanceDynamic> TracksMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	bool bCanShoot;


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for turning input */
	void Turn(const FInputActionValue& Value);
	void TurnStarted(const FInputActionValue& InputActionValue);
	void TurnCompleted(const FInputActionValue& InputActionValue);

	void Shoot(const FInputActionValue& InputActionValue);

	void HandbrakeStarted(const FInputActionValue& InputActionValue);
	void HandbrakeEnded(const FInputActionValue& InputActionValue);

	void MouseWheelUp(const FInputActionValue& InputActionValue);
	void MouseWheelDown(const FInputActionValue& InputActionValue);

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
	USceneComponent* GetShootSocket() const;

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

};
