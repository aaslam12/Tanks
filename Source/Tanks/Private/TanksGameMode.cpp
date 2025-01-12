// Fill out your copyright notice in the Description page of Project Settings.


#include "TanksGameMode.h"

#include "Components/TankHealthComponent.h"
#include "Components/TankSpawnManagerComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/TankGameState.h"
#include "GameFramework/TankPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/TankEnumLibrary.h"
#include "Projectiles/ProjectilePool.h"

ATanksGameMode::ATanksGameMode() : GameStartDelay(3),
                                   SpawnManager(CreateDefaultSubobject<UTankSpawnManagerComponent>("SpawnManager"))
{
}

void ATanksGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	bUseSeamlessTravel = true;
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

	PlayerControllers.Add({ NewPlayerController, FTimerHandle() });

	TankGameState->AssignPlayerToTeam(NewPlayer->PlayerState);
}

void ATanksGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnManager->SetDefaults();
}

void ATanksGameMode::StartMatch()
{
	UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::StartMatch) Setting timer for %.5f"), MinRespawnDelay + 5);

	if (!GameStartingTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(GameStartingTimerHandle, [this]()
		{

		   for (auto PlayerController : PlayerControllers)
		   {
		       if (!PlayerController.Key)
		           continue;

		       UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::StartMatch) GameStartingTimerHandle Timer ran in %s"), *PlayerController.Key->GetName());
		       SetupPawn(PlayerController.Key);

		       PlayerController.Key->SetInputMode(FInputModeGameOnly());
		       PlayerController.Key->bShowMouseCursor = false;
		       PlayerController.Key->EnableInput(PlayerController.Key);
		   }

		   Super::StartMatch();
		},
		GameStartDelay, false);

		UE_LOG(LogTemp, Log, TEXT("(ATanksGameMode::StartMatch) Timer set."));
	}
}

void ATanksGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

FTimerHandle ATanksGameMode::GetPlayerTimerHandle(APlayerState* PlayerState)
{
	if (!PlayerState)
		return FTimerHandle();

	if (!PlayerState->GetPlayerController())
		return FTimerHandle();
		
	auto TimerRef = PlayerControllers.FindByPredicate([PlayerState](const TTuple<APlayerController*, FTimerHandle>& Key)
	{
		return PlayerState->GetPlayerController() == Key.Key;
	});

	if (TimerRef)
		TimerRef->Value.Invalidate();

	return TimerRef ? TimerRef->Value : FTimerHandle(); // should never actually be null
}

void ATanksGameMode::OnPlayerDie(APlayerState* AffectedPlayerState, bool bSelfDestruct)
{
	FTimerHandle TimerHandle = GetPlayerTimerHandle(AffectedPlayerState);
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AffectedPlayerState]
	{
		MC_OnPlayerDie(AffectedPlayerState);
	},
	MinRespawnDelay, false);
}

void ATanksGameMode::MC_OnPlayerDie_Implementation(APlayerState* AffectedPlayerState)
{
	if (!AffectedPlayerState)
		return;
	
	if (!Cast<ATankPlayerState>(AffectedPlayerState))
		return;
	
	AActor* SpawnPoint = SpawnManager->GetRandomSpawnPointFromTeam(Cast<ATankPlayerState>(AffectedPlayerState)->GetCurrentTeam());
	if (SpawnPoint)
	{
		AffectedPlayerState->GetPawn()->Restart();
		RestartPlayerAtPlayerStart(AffectedPlayerState->GetPlayerController(), SpawnPoint);

		AffectedPlayerState->GetPawn()->TeleportTo(
			SpawnPoint->GetActorLocation(),
			SpawnPoint->GetActorRotation()
		);

		DrawDebugSphere(GetWorld(), SpawnPoint->GetActorLocation(), 50, 16, FColor::Emerald, true);
	}
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
