// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Libraries/TankEnumLibrary.h"
#include "GameFramework/PlayerState.h"
#include "TankPlayerState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API ATankPlayerState : public APlayerState
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentTeam();

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnRep_PlayerName() override;

public:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentTeam, BlueprintReadWrite, Category="Teams")
	ETeam CurrentTeam;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	ETeam GetCurrentTeam() const { return CurrentTeam; }

	UFUNCTION(BlueprintCallable, Category="Data")
	void SetCurrentTeam(const ETeam NewTeam);
	
	UFUNCTION(Server, Reliable)
	void SR_SetCurrentTeam(const ETeam NewTeam);

	UFUNCTION(NetMulticast, Reliable)
	void MC_SetCurrentTeam(const ETeam NewTeam);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	FString CustomPlayerName;
};
