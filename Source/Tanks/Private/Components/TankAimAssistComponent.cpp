// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankAimAssistComponent.h"

#include "TankCharacter.h"


void UTankAimAssistComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
		if (Cast<ATankCharacter>(GetOwner()))
			TankCharacter = Cast<ATankCharacter>(GetOwner());
}

// Sets default values for this component's properties
UTankAimAssistComponent::UTankAimAssistComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UTankAimAssistComponent::AimAssist_Implementation(AActor* const LockedTarget)
{
	if (!TankCharacter || !LockedTarget)
		return;

	
}
