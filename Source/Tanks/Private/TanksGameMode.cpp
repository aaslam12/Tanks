// Fill out your copyright notice in the Description page of Project Settings.


#include "TanksGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Projectiles/ProjectilePool.h"

void ATanksGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AProjectilePool::StaticClass(),
		OutActors
	);

	for (auto OutActor : OutActors)
		if (UKismetSystemLibrary::IsValid(OutActor))
			OutActor->Destroy();

	auto SpawnLocation = FVector::ZeroVector;
	auto SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	auto SpawnedActor = GetWorld()->SpawnActor(
		ProjectilePoolClass,
		&SpawnLocation,
		&SpawnRotation
	);

	if (SpawnedActor)
		if (Cast<AProjectilePool>(SpawnedActor))
			ProjectilePool = Cast<AProjectilePool>(SpawnedActor);
}

void ATanksGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	if (!NewPlayer)
		return;

	if (!Cast<APlayerController>(NewPlayer))
		return;

	PlayerControllers.Add(Cast<APlayerController>(NewPlayer));

	// do not spawn another pawn if a pawn already exists for it
	if (NewPlayer->GetPawn())
		return;

	auto SpawnLocation = FVector(PlayerControllers.Num() * -1200.0, 0, 70);
	auto SpawnRotation = FRotator(0, 0, 0);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto SpawnedActor = GetWorld()->SpawnActor(
		DefaultPawnClass,
		&SpawnLocation,
		&SpawnRotation
	);

	NewPlayer->Possess(Cast<APawn>(SpawnedActor));
}
