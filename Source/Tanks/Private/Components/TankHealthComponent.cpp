// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankHealthComponent.h"

#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UTankHealthComponent::UTankHealthComponent(): bShouldRespawn(true), DefaultSelfDestructDelay(5), MinHealth(0),
                                              MaxHealth(1000),
                                              CurrentHealth(MaxHealth)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTankHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTankHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTankHealthComponent::OnPlayerRespawn_Implementation()
{
	CurrentHealth = MaxHealth;
}

void UTankHealthComponent::Die(bool IsSelfDestruct)
{
	if (GetOwner())
		if (Cast<APawn>(GetOwner()))
			OnDieUnreplicated.Broadcast(Cast<APawn>(GetOwner())->GetPlayerState());
	SR_Die(IsSelfDestruct);
}

void UTankHealthComponent::SR_Die_Implementation(bool IsSelfDestruct)
{
	MC_Die(IsSelfDestruct);
}

void UTankHealthComponent::MC_Die_Implementation(bool IsSelfDestruct)
{
	if (!GetOwner())
		return;
	
	if (!Cast<APawn>(GetOwner()))
		return;
	
	OnDie.Broadcast(Cast<APawn>(GetOwner())->GetPlayerState(), IsSelfDestruct, bShouldRespawn);
}

void UTankHealthComponent::OnDamaged(AActor* DamagedActor, float Damage, const UDamageType*,
                                     AController* InstigatedBy, AActor* DamageCauser)
{
	const double OldHealth = CurrentHealth;
	SetHealth(GetHealth() - Damage, false);
	OnTakeDamage.Broadcast(OldHealth, CurrentHealth);
}

void UTankHealthComponent::SelfDestruct(float Delay)
{
	if (Delay <= 0)
		Delay = DefaultSelfDestructDelay;
	SR_SelfDestruct(Delay);
}

void UTankHealthComponent::SR_SelfDestruct_Implementation(float Delay)
{
	MC_SelfDestruct(Delay);
}

void UTankHealthComponent::MC_SelfDestruct_Implementation(float Delay)
{
	if (!SelfDestructTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(SelfDestructTimerHandle, [this]
		{
		    SetHealth(0, true);
			SelfDestructTimerHandle.Invalidate();
		}, Delay, false);
		
		OnSelfDestructStarted.Broadcast();
	}
	else
	{
		// cancel the timer
		GetWorld()->GetTimerManager().ClearTimer(SelfDestructTimerHandle); // also invalidates the timer
		SelfDestructTimerHandle.Invalidate();
		OnSelfDestructCancelled.Broadcast();

		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(MC_SelfDestruct) self destruct cancelled")),
		                                  true, true, FLinearColor::Yellow, 25);
	}
}

void UTankHealthComponent::SetHealth(int NewHealth, bool IsSelfDestruct)
{
	CurrentHealth = NewHealth;
 
	OnHealthChanged.Broadcast(NewHealth, false);

	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;

		Die(IsSelfDestruct);
	}
	else
	{
		// GetWorld()->GetTimerManager().SetTimer(StartHealthRegenTimerHandle, this, &UMultiplayerHealthComponent::StartHealthRegen, TimeToStartHealthRegen, false, TimeToStartHealthRegen);
	}
}

int UTankHealthComponent::GetHealth()
{
	return CurrentHealth;
}

bool UTankHealthComponent::IsDead()
{
	return CurrentHealth <= MinHealth;
}
