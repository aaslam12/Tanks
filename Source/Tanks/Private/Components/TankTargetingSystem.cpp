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

	UKismetSystemLibrary::PrintString(GetWorld(),
		FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) PendingTarget %s"), PendingTarget ? *PendingTarget->GetName() : TEXT("NULL")),
		true, true, FLinearColor::White, 0);

	UKismetSystemLibrary::PrintString(GetWorld(),
		FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) LockedTarget %s"), LockedTarget ? *LockedTarget->GetName() : TEXT("NULL")),
		true, true, FLinearColor::White, 0);

	UKismetSystemLibrary::PrintString(GetWorld(),
		FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) ClosestActor %s"), ClosestActor ? *ClosestActor->GetName() : TEXT("NULL")),
		true, true, FLinearColor::White, 0);
	
	if (ClosestActor)
	{
		if (PendingTime == LockAcquireTime || ClosestActor == LockedTarget)
		{
			// locked on
			LockedTarget = ClosestActor;

			PendingTime = LockAcquireTime;
			LostTime = 0;

			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) locked on to %s"), ClosestActor ? *ClosestActor->GetName() : TEXT("NULL")),
				true, true, FLinearColor::Green, 0);
		}
		else if (PendingTime == 0 && LostTime == LockLoseTime && PendingTarget != LockedTarget)
		{
			LockedTarget = nullptr;

			GainingLock(Delta);
		}
		else if (ClosestActor != LockedTarget && LockedTarget != nullptr)
		{
			// losing lock
			LosingLock(Delta);

			PendingTarget = ClosestActor;

			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) losing lock on %s | not looking at locked target"), LockedTarget ? *LockedTarget->GetName() : TEXT("NULL")),
				true, true, FLinearColor::Red, 0);
		}
		else
		{
			// gaining lock
			GainingLock(Delta);

			PendingTarget = ClosestActor;

			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) gaining lock on %s"), ClosestActor ? *ClosestActor->GetName() : TEXT("NULL")),
				true, true, FLinearColor::Green, 0);
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

			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) losing lock from %s"), LockedTarget ? *LockedTarget->GetName() : TEXT("NULL")),
				true, true, FLinearColor::Red, 0);
		}
	}

	UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) PendingTime: (%.3f/%.3f)"), PendingTime, LockAcquireTime),
				true, true, FLinearColor::Green, 0);

	UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) LostTime: (%.3f/%.3f)"), LostTime, LockLoseTime),
				true, true, FLinearColor::Green, 0);

	DebugSphereAboveActor(LockedTarget, FColor::Green, FVector(0,0, 100));
	DebugSphereAboveActor(PendingTarget, FColor::Magenta);
	return LockedTarget;
}
