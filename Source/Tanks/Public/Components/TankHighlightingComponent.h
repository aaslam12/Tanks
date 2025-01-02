// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TankInterface.h"
#include "Components/ActorComponent.h"

#include "TankHighlightingComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TANKS_API UTankHighlightingComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ATankCharacter> TankCharacter;

	UPROPERTY()
	FTimerHandle TimerHandle;

public:
	// Sets default values for this component's properties
	UTankHighlightingComponent();

	/** The Z offset of the "+" trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces", DisplayName="Box Trace Z Offset")
	double BoxTraceZOffset;

	/** How far to check and highlight enemy tanks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	double BoxTraceLength;
	
	/** The vertical component of the "+" trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector VerticalLineTraceHalfSize;

	/** The horizontal component of the "+" trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup|Traces")
	FVector HorizontalLineTraceHalfSize;

	/** How far to check and highlight enemy tanks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setup")
	double FriendHighlightingThreshold;

private:
	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> HighlightedEnemyTanks;
	
	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> CurrentHitResults;

	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> VerticalHits;

	UPROPERTY(meta=(AllowPrivateAccess="true"))
	TArray<FHitResult> HorizontalHits;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	/**
	 * Runs every 5 seconds via timer. Checks the distances between every friendly tank
	 * and if it's farther than a certain threshold, remove the highlights to save performance.
	 */
	UFUNCTION(BlueprintCallable)
	void HighlightFriendlyTanks();

	/** Creates two box traces that combine to create a "+" sign attached to the gun turret. */
	UFUNCTION(BlueprintNativeEvent)
	void HighlightEnemyTanksIfDetected();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	const TArray<FHitResult>& GetHighlightedEnemyTanks() const { return HighlightedEnemyTanks; }
};
