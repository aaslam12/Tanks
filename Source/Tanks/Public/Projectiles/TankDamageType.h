// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "TankDamageType.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UTankDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UTankDamageType(const FObjectInitializer& ObjectInitializer);
};
