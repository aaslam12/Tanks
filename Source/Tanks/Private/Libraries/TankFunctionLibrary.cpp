// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/TankFunctionLibrary.h"

// #include "GameFramework/PlayerState.h"
// #include "GameFramework/TankGameInstance.h"
// #include "Kismet/GameplayStatics.h"

// void UTankFunctionLibrary::GetAllTanksInTeam(UWorld* WorldContextObject, const FString& TeamName, TArray<ATankCharacter*>& FriendlyTanks)
// {
// 	FriendlyTanks.Empty();
//
// 	UTankGameInstance* GameInstance = Cast<UTankGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
// 	if (GameInstance == nullptr)
// 		return;
//
// 	for (auto Element : GameInstance->Teams)
// 	{
// 		if (Element.Key == TeamName)
// 		{
// 			FriendlyTanks.Add(Element.Value.Players[0]->GetPawn())
// 		}
// 	}
// }
