// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankGameInstance.h"

#include "Kismet/KismetSystemLibrary.h"

UTankGameInstance::UTankGameInstance(): CurrentGameMode(EGameMode::FFA), StartingGameMode(EGameMode::FFA)
{
}

void UTankGameInstance::Init()
{
	Super::Init();

	bool bIsClient = false;

	if (bIsClient)
	{
		UE_LOG(LogTemp, Display, TEXT("Connecting to server in a few seconds..."));

		GetWorld()->GetTimerManager().SetTimer(
		  TimerHandle, 
		  this, 
		  &UTankGameInstance::TryConnectToServer, 
		  5.0f,
		  false
		);
	}
}

void UTankGameInstance::TryConnectToServer()
{
	FString HostAddress = TEXT("127.0.0.1"); // CHANGE IP ADDRESS TO TAILSCALE EXIT NODE
	UE_LOG(LogTemp, Display, TEXT("Connecting to server..."));
	ConnectToServer(HostAddress);
}

bool UTankGameInstance::IsInEditor()
{
	return false;
}

void UTankGameInstance::ConnectToServer(const FString& HostAddress)
{
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		UE_LOG(LogTemp, Display, TEXT("HostAddress: %s"), *HostAddress);
		PlayerController->ClientTravel(HostAddress, TRAVEL_Absolute);
		TimerHandle.Invalidate();

		UE_LOG(LogTemp, Display, TEXT("Connected to server..."));
	}
	else
	{
		UKismetSystemLibrary::PrintString(
			  GetWorld(), 
			  TEXT("(UTankGameInstance::ConnectToServer) PlayerController invalid! trying to reconnect..."), 
			  true, 
			  true, 
			  FLinearColor::Red, 
			  1000000
		);
	}
}
