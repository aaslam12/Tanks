// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Libraries/TankStructLibrary.h"
#include "TankGameState.generated.h"

class AProjectilePool;
struct FTeamData;
/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API ATankGameState : public AGameState
{
	GENERATED_BODY()

	void SpawnProjectilePool();
	void RemoveAllProjectilePools() const;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

protected:
	UFUNCTION()
	void OnRep_Teams();
	
public:
	virtual void AssignPlayerToTeam(APlayerState* NewPlayer);
	
	FTeamData* FindTeam(const ETeam TeamName);

	UPROPERTY(ReplicatedUsing=OnRep_Teams, BlueprintReadOnly, Category="Teams")
	TArray<FTeamData> Teams;
	
	// Assign a player to a team
	UFUNCTION(BlueprintCallable, Category="Teams")
	void AssignPlayerToTeam(APlayerState* Player, const ETeam TeamName);

	// Get players in a specific team
	UFUNCTION(BlueprintCallable, Category="Teams")
	const FTeamData& GetPlayersInTeam(const ETeam TeamName);

	// reference to the singular projectile pool in each level
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<AProjectilePool> ProjectilePool;

	// the class that will be spawned before game begins
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<AProjectilePool> ProjectilePoolClass;
};
