// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankTargetingSystem.h"

#include "Kismet/KismetSystemLibrary.h"

UTankTargetingSystem::UTankTargetingSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTankTargetingSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UTankTargetingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If you want locking purely driven by ProcessHitResults, you can leave Tick empty.
	// Otherwise you could time-out stale PendingTarget here, etc.
}

void UTankTargetingSystem::ProcessHitResults(const TArray<FHitResult>& HitResults)
{
	// Find first valid enemy actor in hits
	AActor* FirstEnemy = nullptr;
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor != GetOwner() /* && team-check here */)
		{
			FirstEnemy = HitActor;
			break;
		}

		if (HitActor)
			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) %s"), *HitActor->GetName()),
											  true, true, FLinearColor::Green, 0);
	}

	// If we already have a lock, check if we still see it
	if (LockedTarget)
	{
		if (FirstEnemy == LockedTarget)
		{
			// still locked on target
			LostTime = 0.f;

			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) still locked on target")),
										  true, true, FLinearColor::Green, 0);
		}
		else
		{
			// start losing the lock
			LostTime += GetWorld()->DeltaTimeSeconds;
			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) start losing the lock")),
										  true, true, FLinearColor::Red, 0);
			if (LostTime >= LockLoseTime)
			{
				// finally lose it
				AActor* Old = LockedTarget;
				LockedTarget = nullptr;
				PendingTarget = nullptr;
				PendingTime = 0.f;
				LostTime = 0.f;
				OnTargetLost.Broadcast(Old);

				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) lost lock on target")),
										  true, true, FLinearColor::Red, 0);
			}
		}
		return;
	}

	// No current lock, see if we can start or continue locking a candidate
	if (FirstEnemy)
	{
		if (PendingTarget == FirstEnemy)
		{
			// continue charging lock
			PendingTime += GetWorld()->DeltaTimeSeconds;
			if (PendingTime >= LockAcquireTime)
			{
				// lock acquired
				LockedTarget = PendingTarget;
				OnTargetLocked.Broadcast(LockedTarget);
				PendingTarget = nullptr;
				PendingTime = 0.f;

				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) lock acquired")),
										  true, true, FLinearColor::Yellow, 0);
			}

			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) charging lock")),
										  true, true, FLinearColor::Yellow, 0);
		}
		else
		{
			UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) different candidate, reset: %s != %s"), *PendingTarget.GetName(), *FirstEnemy->GetName()),
										  true, true, FLinearColor::Yellow, 0);
			
			// different candidate, reset
			PendingTarget = FirstEnemy;
			PendingTime = 0.f;
		}
	}
	else
	{
		// no candidate in sight
		PendingTarget = nullptr;
		PendingTime = 0.f;

		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) no candidate in sight")),
										  true, true, FLinearColor::Green, 0);
	}
}
