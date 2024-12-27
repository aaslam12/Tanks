// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/TankProjectile.h"

#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


void ATankProjectile::CreateSystems()
{
	// Register all trail particle/niagara systems
	for (const auto& Element : TrailParticleSystems)
	{
		if (Element.bUseNiagaraSystem)
		{
			auto Comp = NewObject<UNiagaraComponent>(this, UNiagaraComponent::StaticClass(), 
				FName(FString::Printf(TEXT("Trail_NiagaraComponent_%s"), *Element.NiagaraSystem.GetName()))
			);

			Comp->RegisterComponent();
			Comp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			Comp->SetRelativeTransform(Element.RelativeTransform);
			Comp->SetAsset(Element.NiagaraSystem);
			Comp->bAutoActivate = false;
			TrailNiagaraComponents.Add(Comp);
		}
		else
		{
			auto Comp = NewObject<UParticleSystemComponent>(this, UParticleSystemComponent::StaticClass(),
				FName(FString::Printf(TEXT("Trail_ParticleSystemComponent_%s"), *Element.ParticleSystem.GetName()))
			);

			Comp->RegisterComponent();
			Comp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			Comp->SetRelativeTransform(Element.RelativeTransform);
			Comp->SetTemplate(Element.ParticleSystem);
			Comp->bAutoActivate = false;
			TrailParticleComponents.Add(Comp);
		}
	}

	// Register all hit particle/niagara systems
	for (const auto& Element : HitParticleSystems)
	{
		if (Element.bUseNiagaraSystem)
		{
			auto Comp = NewObject<UNiagaraComponent>(this, UNiagaraComponent::StaticClass(),
				FName(FString::Printf(TEXT("Hit_NiagaraComponent_%s"), *Element.NiagaraSystem.GetName()))
			);

			Comp->RegisterComponent();
			Comp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			Comp->SetRelativeTransform(Element.RelativeTransform);
			Comp->SetAsset(Element.NiagaraSystem);
			Comp->bAutoActivate = false;
			HitNiagaraComponents.Add(Comp);
		}
		else
		{
			auto Comp = NewObject<UParticleSystemComponent>(this, UParticleSystemComponent::StaticClass(),
				FName(FString::Printf(TEXT("Hit_ParticleSystemComponent_%s"), *Element.ParticleSystem.GetName()))
			);

			Comp->RegisterComponent();
			Comp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			Comp->SetRelativeTransform(Element.RelativeTransform);
			Comp->SetTemplate(Element.ParticleSystem);
			Comp->bAutoActivate = false;
			HitParticleComponents.Add(Comp);
		}
	}

	Deactivate();
}

// Sets default values
ATankProjectile::ATankProjectile(): ProjectileMovementComponent(
	                                    CreateDefaultSubobject<UProjectileMovementComponent>(
		                                    "ProjectileMovementComponent")),
                                    bIsInUse(false), TimeToLive(5), bUseSkeletalMesh(false), bDebug(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bForceSubStepping = true;
	ProjectileMovementComponent->bInterpMovement = true;

}

void ATankProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CreateSystems();
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
                                bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse,
                                const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	Deactivate();
}

void ATankProjectile::ActivateTrails()
{
	for (auto Element : TrailNiagaraComponents)
		if (Element)
			Element->Activate(true);

	for (auto Element : TrailParticleComponents)
		if (Element)
			Element->Activate(true);
}

void ATankProjectile::DeactivateHitSystems()
{
	for (auto Element : HitNiagaraComponents)
		if (Element)
			Element->Deactivate();

	for (auto Element : HitParticleComponents)
		if (Element)
			Element->Deactivate();
}

void ATankProjectile::DeactivateTrails()
{
	for (auto Element : TrailParticleComponents)
		if (Element)
			Element->Deactivate();
	for (auto Element : TrailNiagaraComponents)
		if (Element)
			Element->Deactivate();
}

void ATankProjectile::ActivateHitSystems()
{
	for (auto Element : HitParticleComponents)
		if (Element)
			Element->Activate(true);
	for (auto Element : HitNiagaraComponents)
		if (Element)
			Element->Activate(true);
}

void ATankProjectile::Activate_Implementation()
{
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	bIsInUse = true;
	
	TimerHandle.Invalidate();
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("Deactivate"), false);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, TimeToLive, false);

	ProjectileMovementComponent->InitialSpeed = 3000.0;

	ActivateTrails();
	DeactivateHitSystems();
}

void ATankProjectile::Deactivate_Implementation()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	bIsInUse = false;
	
	ProjectileMovementComponent->Velocity = FVector(0);

	DeactivateTrails();
	ActivateHitSystems();
}
