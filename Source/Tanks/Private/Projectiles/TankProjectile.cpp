// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/TankProjectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Projectiles/ShootingInterface.h"


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

	CreateMesh();
}

ATankProjectile::~ATankProjectile()
{
	TimerHandle.Invalidate();
}

void ATankProjectile::OnSphereComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Deactivate();
	SpawnHitParticleSystem(Hit.Location);

	if (UKismetSystemLibrary::IsValid(CallbackObject))
		IShootingInterface::Execute_ProjectileHit(CallbackObject, this, HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
	
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(ATankProjectile::OnSphereComponentHit) Projectile Hit: %s"), *OtherActor->GetName()),
			true, true, FLinearColor::Red, 15);

	DrawDebugSphere(GetWorld(),
		NormalImpulse,
		150, 32, FColor::Blue, true);
}

void ATankProjectile::SpawnHitParticleSystem(const FVector& Location)
{
	for (auto Element : HitParticleSystems)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
		                                         Element.ParticleSystem, Location,
		                                         FRotator(0), FVector(1), true,
		                                         EPSCPoolMethod::AutoRelease
		);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Element.NiagaraSystem, Location, FRotator(0),
		                                               FVector(1), true, true, ENCPoolMethod::AutoRelease);
	}
}

void ATankProjectile::SpawnTrails(const FVector& Location)
{
	for (auto Element : TrailParticleSystems)
	{
		UGameplayStatics::SpawnEmitterAttached(Element.ParticleSystem, GetRootComponent(), "", Location,
											   FRotator(0), FVector(1), EAttachLocation::Type::SnapToTarget, true,
											   EPSCPoolMethod::AutoRelease
		);

		UNiagaraFunctionLibrary::SpawnSystemAttached(Element.NiagaraSystem, GetRootComponent(), "", Location,
													 FRotator(0),
													 EAttachLocation::Type::SnapToTarget, true, true,
													 ENCPoolMethod::AutoRelease);
	}
}

void ATankProjectile::CreateMesh()
{
	if (bUseSkeletalMesh)
	{
		SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	
		SkeletalMeshComponent->SetupAttachment(GetRootComponent());
		SkeletalMeshComponent->SetRelativeTransform(MeshTransform);
		SkeletalMeshComponent->SetMobility(EComponentMobility::Type::Movable);

		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
		SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	else
	{
		StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	
		StaticMeshComponent->SetupAttachment(GetRootComponent());
		StaticMeshComponent->SetRelativeTransform(MeshTransform);
		StaticMeshComponent->SetMobility(EComponentMobility::Type::Movable);

		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		StaticMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
		StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}

void ATankProjectile::SetMeshAssets()
{
	if (bUseSkeletalMesh)
	{
		if (SkeletalMeshComponent && SkeletalMesh)
			SkeletalMeshComponent->SetSkinnedAssetAndUpdate(SkeletalMesh);
	}
	else
	{
		if (StaticMeshComponent && SkeletalMesh)
			StaticMeshComponent->SetStaticMesh(StaticMesh);
	}
}

void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovementComponent->StopMovementImmediately();
	ProjectileMovementComponent->Deactivate();

	ResetTransform();
}

void ATankProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// add static/skeletal mesh and emitters to actor after construction
	// we do this after construction because we need the data from BP here, which is not available in the C++ constructor
	SetMeshAssets();
}

void ATankProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsInUse)
		DrawDebugSphere(GetWorld(),
			GetActorLocation(),
			400, 32, FColor::White, false, 0.0f);
}

void ATankProjectile::ResetTransform()
{
	SetActorTransform(FTransform(FRotator(0), FVector(0, 0, -100000)));
	Deactivate();
}

void ATankProjectile::Activate_Implementation()
{
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	ProjectileMovementComponent->Activate(true);
	bIsInUse = true;
	
	TimerHandle.Invalidate();
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("Deactivate"), false);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, TimeToLive, false);

	FVector ForwardDirection = GetActorForwardVector();
	ProjectileMovementComponent->Velocity = ForwardDirection * ProjectileMovementComponent->InitialSpeed;
	ProjectileMovementComponent->UpdateComponentVelocity();
}

void ATankProjectile::Deactivate_Implementation()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	ProjectileMovementComponent->Deactivate();
	bIsInUse = false;

	ProjectileMovementComponent->StopMovementImmediately();
}
