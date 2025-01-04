// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankPlayerState.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

void ATankPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentTeam);
}

void ATankPlayerState::OnRep_CurrentTeam()
{
}

void ATankPlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ATankPlayerState::SetCurrentTeam(const FString& NewTeam)
{
	SR_SetCurrentTeam(NewTeam);
}

void ATankPlayerState::SR_SetCurrentTeam_Implementation(const FString& NewTeam)
{
	MC_SetCurrentTeam(NewTeam);
}

void ATankPlayerState::MC_SetCurrentTeam_Implementation(const FString& NewTeam)
{
	CurrentTeam = NewTeam;
}
