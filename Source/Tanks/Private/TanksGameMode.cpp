// Fill out your copyright notice in the Description page of Project Settings.


#include "TanksGameMode.h"

#include "GameFramework/TankGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Projectiles/ProjectilePool.h"

ATanksGameMode::ATanksGameMode()
{
}

void ATanksGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SpawnProjectilePool();
}

void ATanksGameMode::SpawnProjectilePool()
{
	RemoveAllProjectilePools();

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

void ATanksGameMode::RemoveAllProjectilePools() const
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AProjectilePool::StaticClass(),
		OutActors
	);

	for (auto OutActor : OutActors)
		if (UKismetSystemLibrary::IsValid(OutActor))
			OutActor->Destroy();
}

void ATanksGameMode::SpawnPlayerPawn(AController* NewPlayer) const
{
	auto SpawnLocation = FVector(PlayerControllers.Num() * -1500.0, 0, 150);
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

void ATanksGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	UGameplayStatics::SetViewportMouseCaptureMode(GetWorld(), EMouseCaptureMode::CapturePermanently);

	if (!NewPlayer)
		return;
	
	APlayerController* NewPlayerController = Cast<APlayerController>(NewPlayer);
	if (!NewPlayerController)
		return;

	ATankGameState* TankGameState = GetGameState<ATankGameState>();
	if (!TankGameState)
		return;

	PlayerControllers.Add(NewPlayerController);

	TankGameState->OnPostLogin(NewPlayer->PlayerState);

	// do not spawn another pawn if a pawn already exists for it
	if (!NewPlayer->GetPawn())
		SpawnPlayerPawn(NewPlayer);
}
