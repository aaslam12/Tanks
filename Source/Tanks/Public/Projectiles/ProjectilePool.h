// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectilePool.generated.h"

class ATankProjectile;

UCLASS(Abstract)
class TANKS_API AProjectilePool : public AActor
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	AProjectilePool();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
