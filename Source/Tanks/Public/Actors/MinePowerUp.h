// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerUp.h"
#include "MinePowerUp.generated.h"

UCLASS(Abstract)
class TANKS_API AMinePowerUp : public APowerUp
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMinePowerUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
