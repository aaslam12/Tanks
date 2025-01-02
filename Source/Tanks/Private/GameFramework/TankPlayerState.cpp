// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankPlayerState.h"

#include "Net/UnrealNetwork.h"

void ATankPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamName);
}
