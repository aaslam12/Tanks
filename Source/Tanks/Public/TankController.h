// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankController.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UChaosVehicleMovementComponent;
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

	double PrevTurnInput;

	ATankController();
	virtual void OnConstruction(const FTransform& Transform) override;
	void ClampVehicleSpeed() const;
	void SetDriveTorque(float DecelerationTorque) const;
	void SetDriveTorque(float LeftDecelerationTorque, float RightDecelerationTorque) const;
	void HandleVehicleDeceleration();
	void RefreshTankPlayerState();
	virtual void Tick(float DeltaSeconds) override;
	UFUNCTION(BlueprintCallable)
	void SetDefaults();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	void AddInputMappingContext() const;
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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool CanRegisterInput() const;

	UPROPERTY(BlueprintReadWrite)
	FVector2D LookValues;

	UPROPERTY(BlueprintReadWrite)
	FVector2D MoveValues;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInAir;

public:
	// If false, all input is ignored.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Setup|Controls")
	bool bInputMasterSwitch;

	/// If true, the tank will decelerate when idle.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Setup|Controls")
	bool bDecelerateWhenIdle;

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
	TObjectPtr<UInputAction> SelfDestructAction;

	///////////////////////////////////////////////////////////////////////////////////
	/// Setup
protected:
    // Base maximum speed of the tank
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Controls|Deceleration")
    float BaseMaxSpeed/* = 1000.0f*/;
    
    // Current maximum speed limit for deceleration
    UPROPERTY(BlueprintReadOnly, Category = "Setup|Controls|Deceleration")
    float CurrentMaxSpeedLimit;
    
    // How quickly the speed should decrease during deceleration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Controls|Deceleration")
    float DecelerationRate/* = 100.0f*/;

	// Curve that defines how deceleration is applied based on current speed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Controls|Deceleration")
	TObjectPtr<UCurveFloat> DecelerationCurve;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Setup", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATankCameraManager> TankCameraManagerClass;

	UPROPERTY(EditDefaultsOnly, Category="Setup", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0", UIMax="7", SliderExponent=0.1))
	double ShootTimerDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Controls")
	FVector2D MouseSensitivity;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAlive;

	///////////////////////////////////////////////////////////////////////////////////
	/// Variables 
	
private:
	UPROPERTY(BlueprintReadOnly, Category="Timers", meta = (AllowPrivateAccess = "true"))
	FTimerHandle ShootTimerHandle;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="Default", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ATankCharacter> TankPlayer;

	UPROPERTY(BlueprintReadOnly, Category="Default", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChaosVehicleMovementComponent> VehicleMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category="Default", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosWheeledVehicleMovementComponent;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bStopTurn;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
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
	virtual void Move(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	virtual void SR_Move(double Value);
	UFUNCTION(NetMulticast, Reliable)
	virtual void MC_Move(double Value);
	virtual void Move__Internal(double Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	void Turn__Internal(double Value);

	UFUNCTION(Server, Reliable)
	void SR_Turn(const FInputActionValue& Value);

	UFUNCTION(NetMulticast, Reliable)
	void MC_Turn(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void SR_TurnCompleted(const FInputActionValue& Value);

	UFUNCTION(NetMulticast, Reliable)
	void MC_TurnCompleted(const FInputActionValue& Value);
	
	/** Called for turning input */
	void Turn(const FInputActionValue& Value);
	void TurnCompleted__Internal();
	void TurnCompleted(const FInputActionValue& InputActionValue);

	void Shoot(const FInputActionValue& InputActionValue);

	void SelfDestruct(const FInputActionValue& InputActionValue);

	void MouseWheelUp(const FInputActionValue& InputActionValue);
	void MouseWheelDown(const FInputActionValue& InputActionValue);

	///////////////////////////////////////////////////////////////////////////////////
	/// Functions
	void StartShootTimer();

	///////////////////////////////////////////////////////////////////////////////////
	/// Public functions
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	FVector2D GetMoveValues() const { return MoveValues; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	FVector2D GetLookValues() const { return LookValues; }

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