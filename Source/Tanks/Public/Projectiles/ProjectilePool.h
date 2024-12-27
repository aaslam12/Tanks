// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectilePool.generated.h"

class ATankProjectile;

/**
 *  If manually placed in a level, it will be deleted and another will be created.
 */
UCLASS(Abstract)
class TANKS_API AProjectilePool : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile Pool", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ATankProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile Pool", meta = (AllowPrivateAccess = "true"))
	int PoolSize;

	UPROPERTY(BlueprintReadOnly, Category="Projectile Pool", meta = (AllowPrivateAccess = "true"))
	TArray<ATankProjectile*> PooledActors;

	// Sets default values for this actor's properties
	AProjectilePool();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitPool();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UFUNCTION()
	ATankProjectile* FindFirstAvailableProjectile();
	
	UFUNCTION()
	void SpawnFromPool(const FTransform& SpawnTransform);
};
