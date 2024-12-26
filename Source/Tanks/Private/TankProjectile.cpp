// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/TankProjectile.h"


// Sets default values
ATankProjectile::ATankProjectile(): bIsInUse(false), bUseSkeletalMesh(false), bUseNiagaraSystemForTrail(false),
                                    bUseNiagaraSystemForHit(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATankProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATankProjectile::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	Deactivate();
}

void ATankProjectile::Activate_Implementation()
{
	SetActorTickInterval(0);
	
	if (bUseSkeletalMesh)
		SkeletalMeshComponent->SetVisibility(true);
	else
		StaticMeshComponent->SetVisibility(true);

	if (bUseNiagaraSystemForTrail)
	{
		for (auto Element : TrailNiagaraSystems)
		{
			// Get spawned objects from projectile pool and teleport them here.
			// Then set asset and activate them.
		}
	}
	else
	{
		for (auto Element : TrailParticleSystems)
		{
			// Get spawned objects from projectile pool and teleport them here.
			// Then set asset and activate them.
		}
	}
}

void ATankProjectile::Deactivate_Implementation()
{
	SetActorTickInterval(1);
	
	if (bUseSkeletalMesh)
		SkeletalMeshComponent->SetVisibility(false);
	else
		StaticMeshComponent->SetVisibility(false);

	if (bUseNiagaraSystemForHit)
	{
		for (auto Element : HitNiagaraSystems)
		{
			// Get spawned objects from projectile pool and teleport them here.
			// Then set asset and activate them.
		}
	}
	else
	{
		for (auto Element : HitParticleSystems)
		{
			// Get spawned objects from projectile pool and teleport them here.
			// Then set asset and activate them.
		}
	}
}
