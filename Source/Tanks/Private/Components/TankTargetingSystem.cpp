// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankTargetingSystem.h"

#include "Kismet/KismetSystemLibrary.h"

#define DELTA_S GetWorld()->GetDeltaSeconds()

UTankTargetingSystem::UTankTargetingSystem(): LockedTarget(nullptr), PendingTarget(nullptr)
{
    PrimaryComponentTick.bCanEverTick = false;
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

void UTankTargetingSystem::DebugSphereAboveActor(const AActor* Actor, const FColor& Color, const FVector& Offset) const
{
	if (!Actor)
		return;
	
	const FVector Start = Actor->GetActorLocation() + FVector(0,0, 1000) + Offset;

	DrawDebugSphere(
		GetWorld(),
		Start,
		200,
		16,
		Color,
		false, 0
	);
}

const FHitResult* UTankTargetingSystem::FindClosestTarget(const TArray<FHitResult>& HitResults) const
{
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	double MinDistance = DOUBLE_BIG_NUMBER;
	const FHitResult* MinHit = nullptr;
	int Index = -1;

	if (HitResults.IsEmpty())
		return nullptr;
	
	for (const FHitResult& Hit : HitResults)
	{
		// TODO: add a team check here
		auto Actor = Hit.GetActor();
		auto Dist = FVector::Distance(OwnerLocation, Actor->GetActorLocation());
		
		if (Dist < MinDistance)
		{
			MinDistance = Dist;
			MinHit = &Hit;
			Index == -1 ? Index = 0 : Index++;

			
			UE_LOG(LogTemp, Log, TEXT("Dist < MinDistance %s"), *Actor->GetName())
		}
	}

	// will be null only if HitResults is empty
	return Index == -1 ? nullptr : MinHit;
}

void UTankTargetingSystem::LosingLock(const double Delta)
{
	PendingTime = FMath::Clamp(PendingTime - Delta, 0, LockAcquireTime);
	LostTime = FMath::Clamp(LostTime + Delta, 0, LockLoseTime);
}

void UTankTargetingSystem::GainingLock(const double Delta)
{
	PendingTime = FMath::Clamp(PendingTime + Delta, 0, LockAcquireTime);
	LostTime = 0;
}

AActor* UTankTargetingSystem::ProcessHitResults(const TArray<FHitResult>& HitResults)
{
	const double Delta = GetWorld()->GetDeltaSeconds();
	
	const FHitResult* Closest = FindClosestTarget(HitResults);
	AActor* ClosestActor = nullptr;

	if (Closest)
		ClosestActor = Closest->GetActor();

	if (ClosestActor)
	{
		// is it time to lock OR are we looking at locked target
		if (PendingTime == LockAcquireTime || ClosestActor == LockedTarget)
		{
			// locked on
			LockedTarget = ClosestActor;

			PendingTime = LockAcquireTime;
			LostTime = 0;

			// need to make sure this only get called once
			OnTargetLocked.Broadcast(LockedTarget);
		}
		else if (PendingTime == 0 && LostTime == LockLoseTime && PendingTarget != LockedTarget) // if it is time to lose lock, and if we are not looking at the locked target (looking at another tank or null) 
		{
			LockedTarget = nullptr;

			GainingLock(Delta);
		}
		else if (ClosestActor != LockedTarget && LockedTarget != nullptr)
		{
			// losing lock
			LosingLock(Delta);

			PendingTarget = ClosestActor;
		}
		else
		{
			// gaining lock
			GainingLock(Delta);

			PendingTarget = ClosestActor;
		}
	}
	else
	{
		// losing lock
		LosingLock(Delta);

		if (LostTime == LockLoseTime)
		{
			// need to make sure this only get called once
			OnTargetLost.Broadcast(LockedTarget);

			// lost lock
			LockedTarget = nullptr;
			PendingTarget = nullptr;
		}
	}

	// will still keep this as it will help later
	DebugSphereAboveActor(LockedTarget, FColor::Green, FVector(0,0, 100));
	DebugSphereAboveActor(PendingTarget, FColor::Magenta);
	return LockedTarget;
}
