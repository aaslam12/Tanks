// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankProjectile.generated.h"

class UArrowComponent;
class USphereComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;
class AProjectilePool;
class UNiagaraComponent;

USTRUCT(BlueprintType)
struct FProjectileSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FTransform RelativeTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> ParticleSystem;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUseNiagaraSystem;

	FProjectileSettings() : bUseNiagaraSystem(false) {}
};

UCLASS(Abstract)
class TANKS_API ATankProjectile : public AActor
{
	GENERATED_BODY()

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FTimerHandle TimerHandle;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereCollision;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UObject> CallbackObject;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UArrowComponent> ArrowComponent;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UParticleSystemComponent>> TrailParticleComponents;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UParticleSystemComponent>> HitParticleComponents;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UNiagaraComponent>> TrailNiagaraComponents;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UNiagaraComponent>> HitNiagaraComponents;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Projectile Pool")
	bool bIsInUse;

	/* if a projectile has not hit anything after this time has passed, it will deactivate on its own. */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Projectile Pool")
	double TimeToLive;
	
	/* Set when spawned */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	TObjectPtr<AProjectilePool> ProjectilePool;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	bool bUseSkeletalMesh;
	
	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	TObjectPtr<UStaticMesh> StaticMesh;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	TObjectPtr<USkeletalMesh> SkeletalMesh;
	
	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	/* Please add a variable description */
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	/* Please add a variable description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Static/Skeletal Mesh")
	FTransform MeshTransform;

	/* Will be created in the constructor */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	TArray<FProjectileSettings> TrailParticleSystems;

	/* Will be created in the constructor */
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"), Category="Setup|Particle/Niagara System")
	TArray<FProjectileSettings> HitParticleSystems;

	void SpawnHitParticleSystem(const FVector& Location);
	void SpawnTrails(const FVector& Location);

	UFUNCTION()
	void OnSphereComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	ATankProjectile();
	virtual ~ATankProjectile() override;
	void CreateMesh();
	void SetMeshAssets();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintCallable)
	void ResetTransform();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Activate();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Deactivate();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsUsingSkeletalMesh() const { return bUseSkeletalMesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInUse() const { return bIsInUse; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<UParticleSystemComponent*>&  GetTrailParticleComponents() const { return TrailParticleComponents; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<UParticleSystemComponent*>& GetHitParticleComponents() const { return HitParticleComponents; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<FProjectileSettings>& GetTrailParticleSystems() const { return TrailParticleSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<FProjectileSettings>& GetHitParticleSystems() const { return HitParticleSystems; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USphereComponent* GetSphereCollision() const { return SphereCollision; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AProjectilePool* GetProjectilePool() const { return ProjectilePool; }

	UFUNCTION(BlueprintCallable)
	void SetProjectilePool(AProjectilePool* NewProjectilePool) { this->ProjectilePool = NewProjectilePool; }

	void SetCallbackObject(UObject* NewCallbackObject) { this->CallbackObject = NewCallbackObject; }
};
