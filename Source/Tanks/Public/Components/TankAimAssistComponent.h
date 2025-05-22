// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankAimAssistComponent.generated.h"

class ATankCharacter;
/**
 * Component responsible for providing aim assist functionality for the owning actor.
 * Designed to interact with other systems such as targeting or aiming mechanisms to ease aiming.
 * This component is blueprint-spawnable and can be used in custom classes.
 *
 * Run the UTankAimAssistComponent::AimAssist(...) function.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TANKS_API UTankAimAssistComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category=Tank_References)
	TObjectPtr<ATankCharacter> TankCharacter;

	virtual void BeginPlay() override;
	
public:
	// Sets default values for this component's properties
	UTankAimAssistComponent();

public:
	UFUNCTION(BlueprintNativeEvent)
	void AimAssist(AActor* const LockedTarget);
};
