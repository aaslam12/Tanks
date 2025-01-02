// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/ProjectilePool.h"

#include "Kismet/GameplayStatics.h"
#include "Projectiles/TankProjectile.h"


// Sets default values
AProjectilePool::AProjectilePool(): PoolSize(20)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AProjectilePool::~AProjectilePool()
{
	for (auto Element : PooledActors)
		if (UKismetSystemLibrary::IsValid(Element))
			Element->Destroy();
}

// Called when the game starts or when spawned
void AProjectilePool::BeginPlay()
{
	Super::BeginPlay();
	InitPool();
}

void AProjectilePool::InitPool()
{
	const FVector SpawnLocation = FVector(0, 0, -10000); // -10,000
	const FRotator SpawnRotation = FRotator(0);
	
	for (int i = 0; i < PoolSize; ++i)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		auto SpawnedActor = GetWorld()->SpawnActorDeferred<ATankProjectile>(
			ProjectileClass,
			FTransform(SpawnRotation, SpawnLocation),
			nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		SpawnedActor->SetProjectilePool(this);
		UGameplayStatics::FinishSpawningActor(SpawnedActor, FTransform());

		if (SpawnedActor)
			PooledActors.AddUnique(SpawnedActor);
	}
}

// Called every frame
void AProjectilePool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//
	// for (auto Element : PooledActors)
	// {
	// 	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(AProjectilePool::Tick) %s Projectile Location: %s"), *Element->GetName(), *Element->GetActorLocation().ToString()),
	// 	                                  true, true, FLinearColor::Yellow, 0);
	// }
}

ATankProjectile* AProjectilePool::FindFirstAvailableProjectile()
{
	for (auto Element : PooledActors)
		if (!Element->IsInUse())
			return Element;
	return nullptr;
}

ATankProjectile* AProjectilePool::SpawnFromPool(const FTransform& SpawnTransform, UObject* Object/* = nullptr*/, const double InitialSpeed/* = 50000.0*/)
{
	auto FirstAvailableProjectile = FindFirstAvailableProjectile();
	
	if (FirstAvailableProjectile)
	{
		// UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(AProjectilePool::SpawnFromPool) FindFirstAvailableProjectile: %s"), *FirstAvailableProjectile->GetName()),
		// 	true, true, FLinearColor::Red, 15);
		
		FirstAvailableProjectile->SetActorTransform(SpawnTransform);

		if (UKismetSystemLibrary::IsValid(Object))
			FirstAvailableProjectile->SetCallbackObject(Object);
		
		FirstAvailableProjectile->Activate();
	}
	else
	{
		ensureMsgf(0, TEXT("PROJECTILE POOL IS EITHER EMPTY OR FULL %s"), *GetFullName());
	}

	return FirstAvailableProjectile;
}
