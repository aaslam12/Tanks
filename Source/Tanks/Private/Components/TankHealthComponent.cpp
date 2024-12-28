// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankHealthComponent.h"


// Sets default values for this component's properties
UTankHealthComponent::UTankHealthComponent(): MinHealth(0), MaxHealth(1000), CurrentHealth(MaxHealth)
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

void UTankHealthComponent::Die()
{
	OnDieUnreplicated.Broadcast();
	SR_Die();
}

void UTankHealthComponent::SR_Die_Implementation()
{
	MC_Die();
}

void UTankHealthComponent::MC_Die_Implementation()
{
	OnDie.Broadcast();
}

void UTankHealthComponent::OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                     AController* InstigatedBy, AActor* DamageCauser)
{
	const int OldHealth = CurrentHealth;
	SetHealth(GetHealth() - Damage);
	OnTakeDamage.Broadcast(OldHealth, CurrentHealth);
}

void UTankHealthComponent::SetHealth(int NewHealth)
{
	CurrentHealth = NewHealth;

	OnHealthChanged.Broadcast(NewHealth, false);

	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;

		Die();
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

bool UTankHealthComponent::GetIsDead()
{
	return CurrentHealth <= MinHealth;
}
