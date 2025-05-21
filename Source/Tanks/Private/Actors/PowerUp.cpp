// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/PowerUp.h"

#include "TankInterface.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
APowerUp::APowerUp() : PowerUpType(EPowerUpType::Health), PowerUpDuration(30),
                       StaticMesh(CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh")),
                       SphereCollision(CreateDefaultSubobject<USphereComponent>("SphereCollision"))
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(SphereCollision);
	bReplicates = true;

	// will be set in blueprints
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	SphereCollision->SetSphereRadius(200.0f);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	SphereCollision->SetCollisionObjectType(ECC_GameTraceChannel1);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APowerUp::OnOverlapBegin);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &APowerUp::OnOverlapEnd);

	SetPowerUpDuration();
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

void APowerUp::SetPowerUpDuration()
{
	switch (PowerUpType)
	{
	case EPowerUpType::Speed:
	case EPowerUpType::Health:
	case EPowerUpType::Damage:
	case EPowerUpType::Shield:
		PowerUpDuration = 30.0;
		break;

	case EPowerUpType::Mine:
		PowerUpDuration = -1;
		break;

	case EPowerUpType::Special:
		PowerUpDuration = 5;
		break;

	default:
		PowerUpDuration = 30;
		break;
	}
}

void APowerUp::OnOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	ITankInterface::Execute_PowerUpActivated(OtherActor, PowerUpType);

	PlayActivateAnimation();
	FadeOut();

	UKismetSystemLibrary::PrintString(
		  GetWorld(), 
		  FString::Printf(TEXT("(APowerUp::OnOverlapBegin) %s Actor %s begin overlap"), *GetName(), *OtherActor->GetName()), 
		  true, 
		  true, 
		  FLinearColor::Red, 
		  1000000
	);
}

void APowerUp::OnOverlapEnd_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UKismetSystemLibrary::PrintString(
		  GetWorld(), 
		  FString::Printf(TEXT("(APowerUp::OnOverlapEnd) %s Actor %s end overlap"), *GetName(), *OtherActor->GetName()), 
		  true, 
		  true, 
		  FLinearColor::Red, 
		  1000000
	);
}

void APowerUp::PlayActivateAnimation_Implementation()
{
}

void APowerUp::FadeOut_Implementation()
{
}
