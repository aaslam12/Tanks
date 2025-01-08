// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TanksGameMode.generated.h"

class UTankSpawnManagerComponent;
enum class ETeam : uint8;
class AProjectilePool;

/**
 * The base tanks game mode class
 */
UCLASS(Abstract)
class TANKS_API ATanksGameMode : public AGameMode
{
	GENERATED_BODY()

	ATanksGameMode();
	void SpawnProjectilePool();
	void RemoveAllProjectilePools() const;
	virtual void PostInitializeComponents() override;
	UFUNCTION()
	void SpawnPlayerPawn(AController* NewPlayer) const;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void BeginPlay() override;
	virtual bool ReadyToStartMatch_Implementation() override;

	UFUNCTION()
	void OnPlayerDie(APlayerState* PlayerState);

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TSet<APlayerController*> PlayerControllers;

	UPROPERTY()
	FTimerHandle GameStartingTimerHandle;

public:
	// reference to the singular projectile pool in each level
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<AProjectilePool> ProjectilePool;

	// the class that will be spawned before game begins
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<AProjectilePool> ProjectilePoolClass;

	// reference to the singular projectile pool in each level
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankSpawnManagerComponent> SpawnManager;
};
