// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankSpawnManagerComponent.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/TankEnumLibrary.h"


// Sets default values for this component's properties
UTankSpawnManagerComponent::UTankSpawnManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTankSpawnManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// get all player start points in the map
	TArray<AActor*> PlayerStartPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartPoints);
	FTeam Teams;

	// loop through them all once to get each team's spawn points
	for (auto StartPoint : PlayerStartPoints)
	{
		
		for (ETeam Team : Teams.Teams)
		{
			FString TeamTag = UEnum::GetValueAsString(Team); // Converts enum to string like "ETeam::Team_1"

			// Filter spawn points for the current team
			if (StartPoint && StartPoint->ActorHasTag(FName(*TeamTag)))
			{
			}
		}
	}
}


// Called every frame
void UTankSpawnManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

