// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankTargetingSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLockChanged, AActor*, Target);

/**
 * A system responsible for managing target locking for tanks, using hit results
 * to track and acquire locks on visible targets.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TANKS_API UTankTargetingSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTankTargetingSystem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	AActor* FindClosestTarget(const TArray<AActor*>& HitResults) const;
	void LosingLock(double Delta);
	void GainingLock(double Delta);

	// helps identify which actor we are locking on
	void DebugSphereAboveActor(const AActor* Actor, const FColor& Color, const FVector& Offset = FVector::ZeroVector) const;

public:
	/** Call each tick after your cone trace; supply all hit results */
	UFUNCTION(BlueprintCallable, Category="Target Locking")
	AActor* ProcessHitResults(const TArray<AActor*>& HitResults);

	/** How long we need to see the same target before we call it “locked” */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Target Locking", meta=(UIMin=0.01, ClampMin=0.01))
	float LockAcquireTime = 0.5f;

	/** How long we can lose sight before we drop the lock */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Target Locking", meta=(UIMin=0.01, ClampMin=0.01))
	float LockLoseTime = 0.5f;

	/** Fired once when LockTarget is first reached */
	UPROPERTY(BlueprintAssignable, Category="Target Locking")
	FOnTargetLockChanged OnTargetLocked;

	/** Fired once when we finally lose our lock */
	UPROPERTY(BlueprintAssignable, Category="Target Locking")
	FOnTargetLockChanged OnTargetLost;

	/** The actor we currently have locked (nullptr if none) */
	UPROPERTY(BlueprintReadOnly, Category="Target Locking")
	TObjectPtr<AActor> LockedTarget;

	/** Candidate we’re currently looking at */
	UPROPERTY(BlueprintReadOnly, Category="Target Locking")
	TObjectPtr<AActor> PendingTarget;

protected:
	/** How long we’ve continuously seen PendingTarget */
	UPROPERTY(BlueprintReadOnly, Category="Target Locking", meta=(ClampMin=0))
	float PendingTime = 0.f;

	/** How long we’ve been without seeing LockedTarget */
	UPROPERTY(BlueprintReadOnly, Category="Target Locking", meta=(ClampMin=0))
	float LostTime = 0.f;

private:
	// is true when lock has been gained.
	bool bIsLockedOn;

	// is true when target is locked on or is gaining lock.
	// will be false when either locked target or pending target is null 
	bool bIsGainingLock;

public:
	UFUNCTION(BlueprintCallable, Category=Getters)
	bool IsLockedOn() const { return bIsLockedOn; }

	UFUNCTION(BlueprintCallable, Category=Getters)
	bool IsGainingLock() const { return bIsGainingLock; }
};
