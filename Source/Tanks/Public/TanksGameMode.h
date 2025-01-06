// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TanksGameMode.generated.h"

class AProjectilePool;

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	FFA UMETA(DisplayName = "Free For All"),
	TDM UMETA(DisplayName = "Team Deathmatch"),
};

/**
 * The base tanks game mode class
 */
UCLASS(Abstract)
class TANKS_API ATanksGameMode : public AGameMode
{
	GENERATED_BODY()

	ATanksGameMode();
	void SpawnProjectilePool();
	void RemoveAnyProjectilePoolsPresent() const;
	virtual void PostInitializeComponents() override;
	void SpawnPlayerPawn(AController* NewPlayer) const;
	virtual void OnPostLogin(AController* NewPlayer) override;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TSet<APlayerController*> PlayerControllers;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	EGameMode CurrentGameMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	EGameMode StartingGameMode;

public:
	// reference to the singular projectile pool in each level
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<AProjectilePool> ProjectilePool;

	// the class that will be spawned before game begins
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<AProjectilePool> ProjectilePoolClass;
};
