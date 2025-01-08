// Fill out your copyright notice in the Description page of Project Settings.


#include "TanksGameMode.h"

#include "TankInterface.h"
#include "Components/TankHealthComponent.h"
#include "Components/TankSpawnManagerComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/TankGameState.h"
#include "GameFramework/TankPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/TankEnumLibrary.h"
#include "Projectiles/ProjectilePool.h"

ATanksGameMode::ATanksGameMode() : SpawnManager(CreateDefaultSubobject<UTankSpawnManagerComponent>("SpawnManager"))
{
}

void ATanksGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	bUseSeamlessTravel = true;
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
	if (!NewPlayer)
		return;

	FVector SpawnLocation = FVector(PlayerControllers.Num() * -1500.0, 0, 150);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto SpawnedActor = GetWorld()->SpawnActor(
		DefaultPawnClass,
		&SpawnLocation,
		&SpawnRotation
	);

	NewPlayer->Possess(Cast<APawn>(SpawnedActor));

	ETeam TeamToSpawnIn = ETeam::Unassigned;
	if (NewPlayer->PlayerState)
		if (Cast<ATankPlayerState>(NewPlayer->PlayerState))
			TeamToSpawnIn = Cast<ATankPlayerState>(NewPlayer->PlayerState)  ->  GetCurrentTeam();

	auto PlayerStart = SpawnManager->GetSpawnLocation(TeamToSpawnIn);

	if (PlayerStart)
	{
		SpawnManager->MakePlayerStartUnavailable(PlayerStart, TeamToSpawnIn);

		SpawnLocation = PlayerStart->GetActorLocation();
		SpawnRotation = PlayerStart->GetActorRotation();
		SpawnedActor->TeleportTo(SpawnLocation, SpawnRotation);
		
		UE_LOG(LogTemp, Log, TEXT("PlayerStart: %s"), *PlayerStart->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerStart is null"));
	}

	UE_LOG(LogTemp, Log, TEXT("SpawnPlayerPawn Location: %s Rotation: %s"), *SpawnLocation.ToString(), *SpawnRotation.ToString());
	UE_LOG(LogTemp, Log, TEXT("TeamToSpawnIn: %ls"), *UEnum::GetValueAsString(TeamToSpawnIn));

	if (auto Component = SpawnedActor->FindComponentByClass(UTankHealthComponent::StaticClass()))
		Cast<UTankHealthComponent>(Component)->OnDie.AddDynamic(this, &ATanksGameMode::OnPlayerDie);
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
}

void ATanksGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnManager->SetDefaults();
}

bool ATanksGameMode::ReadyToStartMatch_Implementation()
{
	if (!GameStartingTimerHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(GameStartingTimerHandle, [this]()
		{
			for (auto Element : PlayerControllers)
				// do not spawn another pawn if a pawn already exists for it
					if (!Element->GetPawn())
						SpawnPlayerPawn(Element);
		}, 5, false);
	
	return Super::ReadyToStartMatch_Implementation();
}

void ATanksGameMode::OnPlayerDie(APlayerState* AffectedPlayerState)
{
	FTimerHandle RespawnTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, [this, AffectedPlayerState]()
	{
		// AActor* SpawnPoint = SpawnManager->GetRandomSpawnPoint();
		// if (SpawnPoint)
		// {
		// 	RestartPlayerAtPlayerStart(AffectedPlayerState->GetOwningController(), SpawnPoint);
		// }
	}, MinRespawnDelay, false);
}
