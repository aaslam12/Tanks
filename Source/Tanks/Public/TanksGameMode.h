// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TanksGameMode.generated.h"

class AProjectilePool;
/**
 * The base tanks game mode class
 */
UCLASS(Abstract)
class TANKS_API ATanksGameMode : public AGameMode
{
	GENERATED_BODY()

	virtual void PostInitializeComponents() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	void SpawnPawn(AController* NewPlayer);
	virtual void OnPostLogin(AController* NewPlayer) override;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TSet<APlayerController*> PlayerControllers;

public:
	// reference to the singular projectile pool in each level
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<AProjectilePool> ProjectilePool;

	// the class that will be spawned before game begins
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<AProjectilePool> ProjectilePoolClass;
};
