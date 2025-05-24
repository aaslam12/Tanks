// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankAimAssistComponent.generated.h"

class ATankCharacter;
/**
 * Component responsible for providing aim assist functionality for the owning actor.
 * Designed to interact with other systems such as targeting or aiming mechanisms to ease aiming.
 * This component is blueprint-spawnable and can be used in custom classes.
 *
 * Run the UTankAimAssistComponent::AimAssist(...) function.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TANKS_API UTankAimAssistComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Tank References", meta=(AllowPrivateAccess="true"))
	TObjectPtr<ATankCharacter> TankCharacter;

	virtual void BeginPlay() override;
	
public:
	// Sets default values for this component's properties
	UTankAimAssistComponent();

public:
	UFUNCTION(BlueprintCallable)
	void AimAssist(AActor* const LockedTarget) const;
	
	/** Enables or disables the aim assist functionality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist")
	bool bEnableAimAssist = true;
	
	/** Enables or disables horizontal (turret rotation) aim assist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist")
	bool bEnableHorizontalAssist = true;
	
	/** Enables or disables vertical (gun elevation) aim assist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist")
	bool bEnableVerticalAssist = true;
	
	/** Strength of the horizontal aim assist (0.0 = no assist, 1.0 = full assist) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist", meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float HorizontalAssistStrength = 0.5f;
	
	/** Strength of the vertical aim assist (0.0 = no assist, 1.0 = full assist) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist", meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float VerticalAssistStrength = 0.5f;
};