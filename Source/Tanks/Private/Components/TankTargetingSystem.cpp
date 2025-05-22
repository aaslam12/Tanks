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

void UTankTargetingSystem::DebugSphereAboveActor(const AActor* Closest, const FColor& Color) const
{
	if (!Closest)
		return;
	
	const FVector Start = Closest->GetActorLocation() + FVector(0,0, 1000);

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

AActor* UTankTargetingSystem::ProcessHitResults(const TArray<FHitResult>& HitResults)
{
	const double Delta = GetWorld()->GetDeltaSeconds();
	
	const FHitResult* Closest = FindClosestTarget(HitResults);
	DebugSphereAboveActor(Closest ? Closest->GetActor() : nullptr, FColor::Magenta);

	PendingTarget = Closest ? Closest->GetActor() : nullptr;

	UKismetSystemLibrary::PrintString(GetWorld(),
		FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) PendingTarget %s"), PendingTarget ? *PendingTarget->GetName() : TEXT("NULL")),
		true, true, FLinearColor::White, 0);

	if (PendingTarget && (PendingTarget != LockedTarget || LockedTarget == nullptr))
	{
		// gaining lock
		PendingTime = FMath::Clamp(PendingTime + Delta, 0, LockAcquireTime);
		LostTime = 0;

		if (PendingTime == LockAcquireTime)
		{
			// locked on
			LockedTarget = PendingTarget;

			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) locked on %s"), PendingTarget ? *PendingTarget->GetName() : TEXT("NULL")),
				true, true, FLinearColor::Green, 0);
		}
	}
	else if (PendingTarget == nullptr)
	{
		// losing lock
		PendingTime = FMath::Clamp(PendingTime - Delta, 0, LockAcquireTime);
		LostTime = FMath::Clamp(LostTime + Delta, 0, LockLoseTime);

		if (LostTime == LockLoseTime)
		{
			// lost lock
			LockedTarget = nullptr;

			UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) losing lock %s"), PendingTarget ? *PendingTarget->GetName() : TEXT("NULL")),
				true, true, FLinearColor::Red, 0);
		}
	}

	UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) PendingTime: %f"), PendingTime),
				true, true, FLinearColor::Green, 0);

	UKismetSystemLibrary::PrintString(GetWorld(),
				FString::Printf(TEXT("(UTankTargetingSystem::ProcessHitResults) LostTime: %f"), LostTime),
				true, true, FLinearColor::Green, 0);

	return LockedTarget;
}
