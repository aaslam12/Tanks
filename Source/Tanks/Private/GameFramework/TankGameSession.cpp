// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/TankGameSession.h"


// Sets default values
ATankGameSession::ATankGameSession()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATankGameSession::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATankGameSession::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

