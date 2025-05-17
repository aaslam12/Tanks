// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/PowerUp.h"

#include "TankInterface.h"
#include "Components/SphereComponent.h"


// Sets default values
APowerUp::APowerUp() : StaticMesh(CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh")),
                       SphereCollision(CreateDefaultSubobject<USphereComponent>("SphereCollision"))
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SphereCollision->SetupAttachment(RootComponent);
	StaticMesh->SetupAttachment(SphereCollision);

	// will be set in blueprints
	StaticMesh->SetStaticMesh(nullptr);
	SphereCollision->SetSphereRadius(50.0f);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APowerUp::OnOverlapBegin);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &APowerUp::OnOverlapEnd);
}

// Called when the game starts or when spawned
void APowerUp::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APowerUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APowerUp::OnOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ITankInterface::Execute_PowerUpActivated(OtherActor, PowerUpType);

	PlayActivateAnimation();
	FadeOut();
}

void APowerUp::OnOverlapEnd_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void APowerUp::PlayActivateAnimation_Implementation()
{
}

void APowerUp::FadeOut_Implementation()
{
}
