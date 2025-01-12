// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TanksGameMode.generated.h"

class UTankSpawnManagerComponent;
enum class ETeam : uint8;

/**
 * The base tanks game mode class
 */
UCLASS(Abstract)
class TANKS_API ATanksGameMode : public AGameMode
{
	GENERATED_BODY()

	ATanksGameMode();
	
	virtual void PostInitializeComponents() override;
	UFUNCTION()
	AActor* SpawnPlayerPawn(AController* NewPlayer) const;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void BeginPlay() override;
	virtual void StartMatch() override;
	virtual void HandleMatchHasStarted() override;

	FTimerHandle GetPlayerTimerHandle(APlayerState* PlayerState);
	
	UFUNCTION()
	void OnPlayerDie(APlayerState* AffectedPlayerState, bool bSelfDestruct);

	UFUNCTION(NetMulticast, Reliable)
	void MC_OnPlayerDie(APlayerState* PlayerState);

	void BindDelegates(AActor* SpawnedActor);
	void SetupPawn(APlayerController* PlayerController);

	/**
	 *  Needed because we need TimerHandles for every player.
	 *  We also need to keep this timer in the server to prevent cheating
	 *  (game modes only exist in the server).
	 */
	TArray<TTuple<APlayerController*, FTimerHandle>> PlayerControllers;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FTimerHandle GameStartingTimerHandle;

protected:
	/**
	 * How long the delay should be to wait for players to connect before the game starts.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(UIMin="1", ClampMin="1"), Category="_Setup")
	float GameStartDelay;

	// reference to the singular projectile pool in each level
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTankSpawnManagerComponent> SpawnManager;
};
