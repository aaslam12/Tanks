// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShootingInterface.generated.h"

class ATankProjectile;
// This class does not need to be modified.
UINTERFACE()
class UShootingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TANKS_API IShootingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * This is supposed to be used as a callback function when a projectile you spawned hits something. NOT when a projectile hits you. 
	 * @param TankProjectile The projectile you spawned 
	 * @param HitComponent The hit component of the projectile
	 * @param OtherActor The hit actor
	 * @param OtherComp The hit component
	 * @param NormalImpulse The normal impusle
	 * @param Hit The hitresult
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ProjectileHit(ATankProjectile* TankProjectile, UPrimitiveComponent* HitComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
