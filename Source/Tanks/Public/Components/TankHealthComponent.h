// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankHealthComponent.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, bool, IsRegenerating);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakeDamage, int, OldHealth, int, NewHealth);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelfDestructTriggered);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelfDestructCancelled);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDie, APlayerState*, PlayerState, bool, bSelfDestruct, bool, bShouldRespawn);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDieUnreplicated, APlayerState*, AffectedPlayerState);

/**
 * The base class for the Tank Health Component.
 * Made to be plug and play with any actor, not just tanks.
 */
UCLASS(Blueprintable, ClassGroup=(TankGame))
class TANKS_API UTankHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	// Sets default values for this component's properties
	UTankHealthComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	bool bShouldRespawn;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true", UIMin=0.01, ClampMin=0.01, UIMax=15, EditCondition="bShouldRespawn"))
	double DefaultSelfDestructDelay;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	double MinHealth;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	double MaxHealth;

	// the current health of the player. will start with the max health possible
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Values, meta=(AllowPrivateAccess="true"))
	double CurrentHealth;
	FTimerHandle SelfDestructTimerHandle;

public:
	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnTakeDamage OnTakeDamage;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnDie OnDie;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnSelfDestructTriggered OnSelfDestructStarted;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnSelfDestructCancelled OnSelfDestructCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnDieUnreplicated OnDieUnreplicated;

	UFUNCTION(BlueprintNativeEvent, Category = "Functions")
	void OnPlayerRespawn();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Die(bool IsSelfDestruct);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void SR_Die(bool IsSelfDestruct);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MC_Die(bool IsSelfDestruct);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	/**
	 * Destroys the tank after a delay, then respawns it after the respawn delay has passed.
	 * @param Delay How long the delay should be. if given a value that is less than or equal to 0, it will default to the DefaultSelfDestructDelay member variable
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SelfDestruct(float Delay);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void SR_SelfDestruct(float Delay);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MC_SelfDestruct(float Delay);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHealth(int NewHealth, bool IsSelfDestruct);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetHealth();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool IsDead();
};
