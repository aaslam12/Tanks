// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class ATankCharacter;
class ATankCameraManager;
/**
 * The base class for the tank controllers
 */
UCLASS(Blueprintable)
class TANKS_API ATankController : public APlayerController
{
	GENERATED_BODY()

	ATankController();
	virtual void Tick(float DeltaSeconds) override;
	void SetDefaults();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	UFUNCTION(BlueprintCallable)
	void BindControls();

	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;

	///////////////////////////////////////////////////////////////////////////////////
	/// Input variables
	
	/** MappingContext */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;
	
	/** Move Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TurnAction;

	/** Look Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** Shoot Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	/** Shoot Input Action */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	FTimerHandle ShootTimer;

	/** Shoot Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseWheelUpAction;
	
	/** Shoot Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseWheelDownAction;

	/** Handbrake Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HandbrakeAction;

	///////////////////////////////////////////////////////////////////////////////////
	/// Setup
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATankCameraManager> TankCameraManagerClass;

	UPROPERTY(EditDefaultsOnly, Category="Setup", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0", UIMax="7", SliderExponent=0.1))
	double ShootTimerDuration;

	///////////////////////////////////////////////////////////////////////////////////
	/// Variables 
	
private:
	UPROPERTY(BlueprintReadOnly, Category="Timers", meta = (AllowPrivateAccess = "true"))
	FTimerHandle ShootTimerHandle;
protected:

	UPROPERTY(BlueprintReadOnly, Category="Default", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ATankCharacter> TankPlayer;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bStopTurn;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double VehicleYaw;

	// this is true when the tank is reloading
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	bool bCanShoot;

	// if shooting is physically blocked by own tank. this is true when the gun is adjusting to its new minimum elevation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Default", meta=(AllowPrivateAccess="true"))
	bool bShootingBlocked;

	///////////////////////////////////////////////////////////////////////////////////
	/// Input functions
	
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

	///////////////////////////////////////////////////////////////////////////////////
	/// Public functions
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	FVector2D GetMoveValues() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	FVector2D GetLookValues() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	bool CanShoot() const { return bCanShoot; }

	UFUNCTION(BlueprintCallable, Category="Data")
	void SetCanShoot(bool bCanShootLoc) { this->bCanShoot = bCanShootLoc; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	bool IsShootingBlocked() const { return bShootingBlocked; }

	UFUNCTION(BlueprintCallable, Category="Data")
	void SetShootingBlocked(bool bShootingBlockedLoc) { this->bShootingBlocked = bShootingBlockedLoc; }

	
};
