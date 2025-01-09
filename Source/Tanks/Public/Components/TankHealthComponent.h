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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDie, APlayerState*, PlayerState);

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

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	double MinHealth;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	double MaxHealth;

	// the current health of the player. will start with the max health possible
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Values, meta=(AllowPrivateAccess="true"))
	double CurrentHealth;
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnTakeDamage OnTakeDamage;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnDie OnDie;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnDieUnreplicated OnDieUnreplicated;

	UFUNCTION(BlueprintNativeEvent, Category = "Functions")
	void OnPlayerRespawn();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void SR_Die();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MC_Die();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHealth(int NewHealth);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetHealth();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool IsDead();
};
