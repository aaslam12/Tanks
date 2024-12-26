// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankProjectile.generated.h"

class AProjectilePool;
class UNiagaraComponent;

UCLASS(Abstract)
class TANKS_API ATankProjectile : public AActor
{
	GENERATED_BODY()

	/* Set when spawned */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	TObjectPtr<AProjectilePool> ProjectilePool;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Projectile Pool")
	bool bIsInUse;
	
	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	bool bUseSkeletalMesh;
	
	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	TArray<TObjectPtr<UParticleSystemComponent>> TrailParticleSystems;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	TArray<TObjectPtr<UParticleSystemComponent>> HitParticleSystems;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	TArray<TObjectPtr<UNiagaraComponent>> TrailNiagaraSystems;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	TArray<TObjectPtr<UNiagaraComponent>> HitNiagaraSystems;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	bool bUseNiagaraSystemForTrail;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	bool bUseNiagaraSystemForHit;

	// Sets default values for this actor's properties
	ATankProjectile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Activate();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Deactivate();

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsUsingSkeletalMesh() const { return bUseSkeletalMesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInUse() const { return bIsInUse; }
	
	UFUNCTION(BlueprintCallable)
	void SetInUse(bool bNewInUse) { bIsInUse = bNewInUse; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<TObjectPtr<UParticleSystemComponent>>&  GetTrailParticleSystems() const { return TrailParticleSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<TObjectPtr<UParticleSystemComponent>>& GetHitParticleSystems() const { return HitParticleSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<TObjectPtr<UNiagaraComponent>>& GetTrailNiagaraSystems() const { return TrailNiagaraSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<TObjectPtr<UNiagaraComponent>>& GetHitNiagaraSystems() const { return HitNiagaraSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsUsingNiagaraSystem() const { return bUseNiagaraSystemForTrail; }
};
