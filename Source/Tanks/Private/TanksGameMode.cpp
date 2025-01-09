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

AActor* ATanksGameMode::SpawnPlayerPawn(AController* NewPlayer) const
{
	if (!NewPlayer)
		return nullptr;

	FVector SpawnLocation = FVector(PlayerControllers.Num() * -1500.0, 0, 150);
	FRotator SpawnRotation = FRotator::ZeroRotator;

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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerStart is null"));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto SpawnedActor = GetWorld()->SpawnActor(
		DefaultPawnClass,
		&SpawnLocation,
		&SpawnRotation
	);

	return SpawnedActor;
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

	UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::OnPostLogin) Setting timer for %.5f"), MinRespawnDelay + 5);

	if (!GameStartingTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(GameStartingTimerHandle, [this]()
		{

			for (auto PlayerController : PlayerControllers)
			{
				UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::OnPostLogin) GameStartingTimerHandle Timer ran in %s"), *PlayerController->GetName());
				SetupPawn(PlayerController);
			}
		}, MinRespawnDelay + 3, false);

		UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::OnPostLogin) Timer set."));
	}
}

void ATanksGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnManager->SetDefaults();
}

void ATanksGameMode::OnPlayerDie(APlayerState* AffectedPlayerState)
{
	UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::OnPlayerDie) AffectedPlayerState: %s Setting timer for %.5f"), *AffectedPlayerState->GetName(), MinRespawnDelay);
	
	FTimerHandle RespawnTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, [this, AffectedPlayerState]()
	{
		UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::OnPlayerDie) Timer ran..."));

		if (AffectedPlayerState)
			if (Cast<ATankPlayerState>(AffectedPlayerState))
			{
				AActor* SpawnPoint = SpawnManager->GetRandomSpawnPointFromTeam(Cast<ATankPlayerState>(AffectedPlayerState)->GetCurrentTeam());
				if (SpawnPoint)
				{
					AffectedPlayerState->GetPawn()->Restart();
					RestartPlayerAtPlayerStart(AffectedPlayerState->GetPlayerController(), SpawnPoint);
				}
			}
	}, MinRespawnDelay, false);
	
	UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::OnPlayerDie) Timer set."));

}

void ATanksGameMode::BindDelegates(AActor* SpawnedActor)
{
	if (auto Component = SpawnedActor->FindComponentByClass(UTankHealthComponent::StaticClass()))
		Cast<UTankHealthComponent>(Component)->OnDie.AddDynamic(this, &ATanksGameMode::OnPlayerDie);
}

void ATanksGameMode::SetupPawn(APlayerController* PlayerController)
{
	AActor* SpawnedActor = nullptr;
				
	// do not spawn another pawn if a pawn already exists for it
	if (!PlayerController->GetPawn())
	{
		SpawnedActor = SpawnPlayerPawn(PlayerController);
	}
	else
	{
		// already exists so no need to spawn a pawn. just bind the delegates.
		BindDelegates(PlayerController->GetPawn());
	}

	if (!SpawnedActor)
		return;
					
	PlayerController->Possess(Cast<APawn>(SpawnedActor));

	BindDelegates(SpawnedActor);
}
