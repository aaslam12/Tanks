// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Libraries/TankStructLibrary.h"
#include "TankGameInstance.generated.h"

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	FFA UMETA(DisplayName = "Free For All"),
	TDM UMETA(DisplayName = "Team Deathmatch"),
};

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UTankGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UTankGameInstance();
	virtual void Init() override;
	void ConnectToServer(const FString& HostAddress);

public:
	UPROPERTY(BlueprintReadOnly)
	FTimerHandle TimerHandle;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	EGameMode CurrentGameMode;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	EGameMode StartingGameMode;

	UFUNCTION(BlueprintCallable)
	void TryConnectToServer();

	UFUNCTION(BlueprintCallable)
	bool IsInEditor();

};
