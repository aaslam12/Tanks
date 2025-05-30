// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankTargetingSystem.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UTankTargetingSystem::UTankTargetingSystem(): LockAcquireTime(0.5f), LockLoseTime(0.5f),
                                              LockedTarget(nullptr),
                                              PendingTarget(nullptr),
                                              bIsLockedOn(false),
                                              bIsGainingLock(false), bIsReseting(false), bCanLockOn(true)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTankTargetingSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UTankTargetingSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (OnTargetLocked.IsBound())
		OnTargetLocked.Clear();

	if (OnTargetLost.IsBound())
		OnTargetLost.Clear();
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

AActor* UTankTargetingSystem::FindClosestTarget(const TArray<AActor*>& HitResults) const
{
	double StartTime = FPlatformTime::Seconds();
	
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	AActor* ClosestActor = nullptr;

	// copied and slightly modified from UGameplayStatics::FindNearestActor(...) 
	float DistanceFromNearestActor = TNumericLimits<float>::Max();
	for (AActor* ActorToCheck : HitResults)
	{
		if (ActorToCheck)
		{
			const float DistanceFromActorToCheck = (OwnerLocation - ActorToCheck->GetActorLocation()).SizeSquared();
			if (DistanceFromActorToCheck < DistanceFromNearestActor)
			{
				ClosestActor = ActorToCheck;
				DistanceFromNearestActor = DistanceFromActorToCheck;
			}
		}
	}

	// Optionally print timing here if profiling
	double EndTime = FPlatformTime::Seconds();
	double Duration = (EndTime - StartTime) * 1000.0;
	UKismetSystemLibrary::PrintString(
		GetWorld(), FString::Printf(TEXT("FindClosestTarget: %.3f ms"), Duration), true, false, FLinearColor::Green, 0);

	// will be null only if HitResults is empty
	return ClosestActor;
}

void UTankTargetingSystem::LosingLock(const double Delta)
{
	PendingTime = FMath::Clamp(PendingTime - Delta, 0, LockAcquireTime);
	LostTime = FMath::Clamp(LostTime + Delta, 0, LockLoseTime);

	bIsGainingLock = false;
}

void UTankTargetingSystem::GainingLock(const double Delta)
{
	PendingTime = FMath::Clamp(PendingTime + Delta, 0, LockAcquireTime);
	LostTime = 0;

	bIsGainingLock = true;
}

void UTankTargetingSystem::ResetLock()
{
	LockedTarget = nullptr;
	PendingTarget = nullptr;
	PendingTime = 0;
	LostTime = 0;
}

AActor* UTankTargetingSystem::ProcessHitResults(const TArray<AActor*>& HitResults)
{
	if (bCanLockOn == false)
	{
		ResetLock();
		return nullptr;
	}
	
	const double Delta = GetWorld()->GetDeltaSeconds();

	// Keep the old target around so we can detect transitions
	AActor* OldLockedTarget = LockedTarget;
	AActor* ClosestActor = FindClosestTarget(HitResults);

	if (ClosestActor)
	{
		// is it time to lock OR are we looking at locked target
		if (PendingTime == LockAcquireTime || ClosestActor == LockedTarget)
		{
			// locked on
			LockedTarget = ClosestActor;
			PendingTarget = LockedTarget;
			bIsLockedOn = true;

			PendingTime = LockAcquireTime;
			LostTime = 0;

			// is only called once
			if (OldLockedTarget != LockedTarget)
			{
				OnTargetLocked.Broadcast(LockedTarget);
			}
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
			// lost lock
			LockedTarget = nullptr;
			PendingTarget = nullptr;
			bIsLockedOn = false;

			// is only called once
			if (OldLockedTarget != LockedTarget)
			{
				OnTargetLost.Broadcast(OldLockedTarget);
			}
		}
	}

	// will still keep this as it will help later
	DebugSphereAboveActor(LockedTarget, FColor::Green, FVector(0,0, 100));
	DebugSphereAboveActor(PendingTarget, FColor::Magenta);
	return LockedTarget;
}
