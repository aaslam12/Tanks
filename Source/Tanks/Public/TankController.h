// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankController.generated.h"

class ATankProjectile;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class ATankCharacter;
class ATankCameraManager;

UDELEGATE(DisplayName="On Shoot Event")
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShootEvent);

/**
 * The base class for the tank controllers
 */
UCLASS(Abstract)
class TANKS_API ATankController : public APlayerController
{
	GENERATED_BODY()

	bool bIsAlive;

	ATankController();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	void SetDefaults();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	void SetupInput();

public:
	/** Runs after the shoot input has been processed */
	UPROPERTY(BlueprintAssignable, Category = "Delegate Functions")
	FOnShootEvent OnShoot;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Delegate Functions")
	void OnDie();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Delegate Functions")
	void OnRespawn();

protected:
	UFUNCTION(BlueprintCallable)
	void BindControls();
	bool CanRegisterInput() const;

	UPROPERTY(BlueprintReadOnly)
	FVector2D LookValues;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MoveValues;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInAir;

private:
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
	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
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
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATankCameraManager> TankCameraManagerClass;

	UPROPERTY(EditDefaultsOnly, Category="Setup", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0", UIMax="7", SliderExponent=0.1))
	double ShootTimerDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Controls")
	FVector2D MouseSensitivity;

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
	/// Functions
	void StartShootTimer();

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

	UFUNCTION(BlueprintImplementableEvent)
	void DrawDebugCircle(const FVector2D& Position);
};
