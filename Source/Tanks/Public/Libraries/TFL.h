// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TFL.generated.h"

class ATankCharacter;
/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UTFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Ripped from UGameplayStatics::ApplyRadialDamageWithFalloff and modified it.
	  * Hurt locally authoritative actors within the radius. Will only hit components that block the Visibility channel.
	  * @param BaseDamage - The base damage to apply, i.e. the damage at the origin.
	  * @param MinimumDamage - The minimum damage
	  * @param Origin - Epicenter of the damage area.
	  * @param DamageInnerRadius - Radius of the full damage area, from Origin
	  * @param DamageOuterRadius - Radius of the minimum damage area, from Origin
	  * @param DamageFalloffExponent - Falloff exponent of damage from DamageInnerRadius to DamageOuterRadius
	  * @param DamageTypeClass - Class that describes the damage that was done.
	  * @param IgnoreActors - List of Actors to ignore
	  * @param HitActors - List of actors applied damage to and how much
	  * @param DamageCauser - Actor that actually caused the damage (e.g. the grenade that exploded)
	  * @param InstigatedByController - Controller that was responsible for causing this damage (e.g. player who threw the grenade)
	  * @param DamagePreventionChannel - Damage will not be applied to victim if there is something between the origin and the victim which blocks traces on this channel
	  * @return true if damage was applied to at least one actor.
	 */
	static bool ApplyRadialDamageWithFalloff(const UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloffExponent, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, TArray<TTuple<AActor*, double>>& HitActors, AActor* DamageCauser = NULL, AController* InstigatedByController = NULL, ECollisionChannel DamagePreventionChannel = ECC_Visibility);
};
