// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUp.generated.h"

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

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	EPowerUpType PowerUpType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Components, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereCollision;

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintNativeEvent)
	void PlayActivateAnimation();
	
	UFUNCTION(BlueprintNativeEvent)
	void FadeOut();
	
public:
	APowerUp();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
