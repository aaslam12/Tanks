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

public:
	// Sets default values for this actor's properties
	ATanksCharacter();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<UTankAnimInstance> TankAnimInstanceClass;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTankAnimInstance> AnimInstance;

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


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for turning input */
	void Turn(const FInputActionValue& Value);

	void TurnStarted(const FInputActionValue& InputActionValue);
	
	void TurnCompleted(const FInputActionValue& InputActionValue);

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

};
