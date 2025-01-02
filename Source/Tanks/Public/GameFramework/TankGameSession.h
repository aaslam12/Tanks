// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "TankGameSession.generated.h"

UCLASS(Blueprintable)
class TANKS_API ATankGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATankGameSession();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
