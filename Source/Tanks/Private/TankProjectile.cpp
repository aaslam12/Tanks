// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/TankProjectile.h"

#include "NiagaraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ATankProjectile::ATankProjectile(): SphereCollision(CreateDefaultSubobject<USphereComponent>("SphereCollision")),
									ArrowComponent(CreateDefaultSubobject<UArrowComponent>("Arrow")),
									ProjectileMovementComponent(
										CreateDefaultSubobject<UProjectileMovementComponent>(
											"ProjectileMovementComponent")),
                                    bIsInUse(false), TimeToLive(5), bUseSkeletalMesh(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(200);
	SphereCollision->SetMobility(EComponentMobility::Type::Movable);
	
	SphereCollision->OnComponentHit.AddDynamic(this, &ATankProjectile::OnSphereComponentHit);

	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	ProjectileMovementComponent->UpdatedComponent = SphereCollision;
	ProjectileMovementComponent->InitialSpeed = 50000;
	ProjectileMovementComponent->bThrottleInterpolation = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->bForceSubStepping = true;
	ProjectileMovementComponent->bInterpMovement = true;

	ArrowComponent->ArrowSize = 16.5;
	ArrowComponent->ArrowLength = 128;
}

ATankProjectile::~ATankProjectile()
{
	// just here to stop the errors you get in debugger but it doesnt seem to help
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
	ActivateHitSystems(Hit.Location);
}

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

			Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Comp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
			Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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

			Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Comp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
			Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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

			Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Comp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
			Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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

			Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Comp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
			Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			HitParticleComponents.Add(Comp);
		}
	}

	Deactivate();
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
		Comp->SetMobility(EComponentMobility::Type::Movable);

		Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Comp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
		Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	else
	{
		auto Comp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(),
		                                                           FName(FString::Printf(TEXT("StaticMesh"))));

		Comp->RegisterComponent();
		Comp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Comp->SetRelativeTransform(MeshTransform);
		Comp->SetStaticMesh(StaticMesh);
		Comp->SetMobility(EComponentMobility::Type::Movable);

		Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Comp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
		Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	}
}

void ATankProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// add static/skeletal mesh and emitters to actor after construction
	// we do this after construction because we need the data from BP here, which is not available in the C++ constructor
	CreateMesh();
	CreateSystems();
}

void ATankProjectile::ActivateTrails_Implementation()
{
	for (auto Element : TrailNiagaraComponents)
		if (Element)
			Element->Activate(true);

	for (auto Element : TrailParticleComponents)
		if (Element)
			Element->Activate(true);
}

void ATankProjectile::DeactivateHitSystems_Implementation()
{
	for (auto Element : HitNiagaraComponents)
		if (Element)
			Element->Deactivate();

	for (auto Element : HitParticleComponents)
		if (Element)
			Element->Deactivate();
}

void ATankProjectile::DeactivateTrails_Implementation()
{
	for (auto Element : TrailParticleComponents)
		if (Element)
			Element->Deactivate();
	for (auto Element : TrailNiagaraComponents)
		if (Element)
			Element->Deactivate();
}

void ATankProjectile::ActivateHitSystems_Implementation(const FVector& HitLocation)
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

	FVector ForwardDirection = GetActorForwardVector();
	ProjectileMovementComponent->Velocity = ForwardDirection * ProjectileMovementComponent->InitialSpeed;
	ProjectileMovementComponent->UpdateComponentVelocity();

	ActivateTrails();
	DeactivateHitSystems();
}

void ATankProjectile::Deactivate_Implementation()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	bIsInUse = false;
	
	ProjectileMovementComponent->StopMovementImmediately();

	DeactivateTrails();
}
