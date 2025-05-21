// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUp.generated.h"

class UNiagaraSystem;
class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EPowerUpType : uint8
{
	Health,
	Shield,
	Damage,
	Speed,
	Mine,
	
	Special,
};

UENUM(BlueprintType)
enum class EPowerUpEffectType : uint8
{
	ParticleSystem,
	NiagaraSystem,
};

/**
 * @struct FPowerUpEffect
 * @brief Defines the effects associated with a power-up in the game.
 *
 * This structure specifies the type of effect, along with optional visual and
 * auditory components, that can be activated when a power-up is used.
 */
USTRUCT(BlueprintType)
struct FPowerUpEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	EPowerUpEffectType PowerUpEffectType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(ToolTip="Optional: You can add a sound here or in the effect system.."))
	TObjectPtr<USoundCue> Sound;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(EditConditionHides, EditCondition="PowerUpEffectType == EPowerUpEffectType::ParticleSystem"))
	TObjectPtr<UParticleSystem> ParticleSystem;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(EditConditionHides, EditCondition="PowerUpEffectType == EPowerUpEffectType::NiagaraSystem"))
	TObjectPtr<UNiagaraSystem> NiagaraSystem;
};

/**
 * @class APowerUp
 * @brief Represents a base class for power-up objects in the game.
 *
 * This class serves as a blueprint for power-up items that players can activate.
 *
 * Subclasses are intended to inherit from this base class to implement specific types of
 * power-up behavior.
 */
UCLASS(Abstract)
class TANKS_API APowerUp : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup)
	EPowerUpType PowerUpType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, DisplayName="Power Up Duration (in seconds)", meta=(ToolTip="Duration is in seconds.", SliderExponent=1, UIMin=0.01, UIMax=50, ClampMin=0.01))
	double PowerUpDuration;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup)
	TArray<FPowerUpEffect> PowerUpEffects;
	


	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup)
	bool bUseStaticMesh;
	
	UPROPERTY(BlueprintReadOnly, Instanced, Category = Components)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(BlueprintReadOnly, Instanced, Category = Components)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(BlueprintReadOnly, Instanced, Category = Components)
	TObjectPtr<USphereComponent> SphereCollision;

	

	// Called when the sphere overlap starts
	UFUNCTION(BlueprintNativeEvent)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when the sphere overlap ends
	UFUNCTION(BlueprintNativeEvent)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Optional function to play an animation when the powerup is activated
	UFUNCTION(BlueprintNativeEvent)
	void PlayActivateAnimation();

	// Optional function to make the powerup fade out or disappear
	UFUNCTION(BlueprintNativeEvent)
	void FadeOut();
	
public:
	void SetPowerUpDuration();

	UFUNCTION(blueprintCallable, BlueprintNativeEvent)
	void Activate();
	APowerUp();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
