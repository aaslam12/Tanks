// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/TankProjectile.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Projectiles/ProjectilePool.h"


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
			
			if (bUseSkeletalMesh)
				Comp->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			else
				Comp->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			
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
			
			if (bUseSkeletalMesh)
				Comp->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			else
				Comp->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			
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
			
			if (bUseSkeletalMesh)
				Comp->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			else
				Comp->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			
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
			
			if (bUseSkeletalMesh)
				Comp->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			else
				Comp->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
			
			Comp->SetRelativeTransform(Element.RelativeTransform);
			Comp->SetTemplate(Element.ParticleSystem);
			Comp->bAutoActivate = false;
			HitParticleComponents.Add(Comp);
		}
	}

	Deactivate();
}

// Sets default values
ATankProjectile::ATankProjectile(): SphereCollision(CreateDefaultSubobject<USphereComponent>("SphereCollision")),
									ProjectileMovementComponent(
										CreateDefaultSubobject<UProjectileMovementComponent>(
											"ProjectileMovementComponent")),
                                    bIsInUse(false), TimeToLive(5), bUseSkeletalMesh(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bForceSubStepping = true;
	ProjectileMovementComponent->bInterpMovement = true;

	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(850);
	SphereCollision->SetSimulatePhysics(true);
	SphereCollision->SetLinearDamping(20);
	SphereCollision->SetCollisionProfileName("PhysicsActor");
	SphereCollision->OnComponentHit.AddDynamic(this, &ATankProjectile::OnSphereComponentHit);
}

ATankProjectile::~ATankProjectile()
{
	for (auto Component : TrailParticleComponents)
		if (UKismetSystemLibrary::IsValid(Component))
			Component->DestroyComponent();

	for (auto Component : HitParticleComponents)
		if (UKismetSystemLibrary::IsValid(Component))
			Component->DestroyComponent();

	for (auto Component : TrailNiagaraComponents)
		if (UKismetSystemLibrary::IsValid(Component))
			Component->DestroyComponent();

	for (auto Component : HitNiagaraComponents)
		if (UKismetSystemLibrary::IsValid(Component))
			Component->DestroyComponent();

	TimerHandle.Invalidate();
}

void ATankProjectile::OnSphereComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Deactivate();

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankProjectile::OnSphereComponentHit) Deactivate() called")),
												  true, true, FLinearColor::Yellow, 15);
}

void ATankProjectile::CreateMesh()
{
	if (bUseSkeletalMesh)
	{
		auto Comp = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(),
		                                                               FName(FString::Printf(TEXT("SkeletalMesh"))));

		Comp->RegisterComponent();
		Comp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Comp->SetRelativeTransform(MeshTransform);
		Comp->SetSkinnedAssetAndUpdate(SkeletalMesh);
	}
	else
	{
		auto Comp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(),
		                                                           FName(FString::Printf(TEXT("StaticMesh"))));

		Comp->RegisterComponent();
		Comp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Comp->SetRelativeTransform(MeshTransform);
		Comp->SetStaticMesh(StaticMesh);
	}
}

void ATankProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CreateMesh();
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
