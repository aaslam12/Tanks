// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/TankDamageType.h"

static bool bCausedByWorld = false;
static bool bScaleMomentumByMass = false;
static double DamageFalloff = 1.5;

static bool bRadialDamageVelChange = false;
static double DamageImpulse = 1500.0;

static double DestructibleImpulse = 1500.0;
static double DestructibleDamageSpreadScale = 3.0;

UTankDamageType::UTankDamageType(const FObjectInitializer&)
{
}
