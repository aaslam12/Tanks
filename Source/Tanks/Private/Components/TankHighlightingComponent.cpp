// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TankHighlightingComponent.h"

#include "TankCharacter.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/TankGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UTankHighlightingComponent::UTankHighlightingComponent(): BoxTraceZOffset(0),
                                                          BoxTraceLength(8000),
                                                          VerticalLineTraceHalfSize(FVector(10, 10, 250)),
                                                          HorizontalLineTraceHalfSize(FVector(10, 100, 10)),
                                                          FriendHighlightingThreshold(20000)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts
void UTankHighlightingComponent::BeginPlay()
{
	Super::BeginPlay();

	TankCharacter = Cast<ATankCharacter>(GetOwner());

	if (TankCharacter->IsLocallyControlled())
	{
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, TEXT("HighlightFriendlyTanks")); // Bind a function
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 5.0f, true);
	}

}

void UTankHighlightingComponent::HighlightFriendlyTanks()
{
	if (!TankCharacter)
		return;
	
	UTankGameInstance* TankGameInstance = Cast<UTankGameInstance>(GetWorld()->GetGameInstance());
	if (!TankGameInstance)
		return;

	if (TankGameInstance->Teams.IsEmpty())
		return;

	auto CurrentTeam = TankCharacter->GetCurrentTeam();
	
	auto Array = TankGameInstance->Teams[CurrentTeam].Players;

	for (auto Element : Array)
	{
		if (!Element)
			continue;
		
		if (Element == TankCharacter->GetPlayerState())
			continue;

		auto OtherTank = Cast<ATankCharacter>(Element->GetPawn());
		if (OtherTank)
		{
			auto DistanceToOtherTank = FVector::Distance(OtherTank->GetActorLocation(), TankCharacter->GetActorLocation());

			if (DistanceToOtherTank > FriendHighlightingThreshold)
			{
				ITankInterface::Execute_OutlineTank(OtherTank, false);
				
				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankHighlightingComponent::HighlightFriendlyTanks) ITankInterface::Execute_OutlineTank: %d"), false),
					true, true, FLinearColor::Yellow, 15);
			}
			else
			{
				ITankInterface::Execute_OutlineTank(OtherTank, true);
				
				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("(UTankHighlightingComponent::HighlightFriendlyTanks) ITankInterface::Execute_OutlineTank: %d"), true),
					true, true, FLinearColor::Yellow, 15);
			}
		}
	}
}

void UTankHighlightingComponent::HighlightEnemyTanksIfDetected_Implementation()
{
	CurrentHitResults.Empty();

	FVector Start = TankCharacter->GetMesh()->GetSocketTransform("GunShootSocket").GetLocation();
	Start.Z += BoxTraceZOffset;
	FVector End = Start + TankCharacter->GetMesh()->GetSocketQuaternion("GunShootSocket").GetForwardVector() *
		BoxTraceLength;

	// horizontal box trace
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		HorizontalLineTraceHalfSize,
		TankCharacter->GetMesh()->GetSocketRotation("GunShootSocket"),
		{ObjectTypeQuery5},
		false,
		{GetOwner()},
		EDrawDebugTrace::None,
		HorizontalHits,
		true,
		FLinearColor::Yellow
	);

	// vertical box trace
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		VerticalLineTraceHalfSize,
		TankCharacter->GetMesh()->GetSocketRotation("GunShootSocket"),
		{ObjectTypeQuery5},
		false,
		{GetOwner()},
		EDrawDebugTrace::None,
		VerticalHits,
		true,
		FLinearColor::Yellow
	);

	// remove duplicates
	for (auto& Hit : VerticalHits)
		if (!CurrentHitResults.Contains(Hit))
			CurrentHitResults.Add(Hit);
	for (auto& Hit : HorizontalHits)
		if (!CurrentHitResults.Contains(Hit))
			CurrentHitResults.Add(Hit);

	// Removes hit results with actors that are no longer detected by the trace.
	for (auto It = HighlightedEnemyTanks.CreateIterator(); It; ++It)
	{
		if (!CurrentHitResults.Contains(*It))
		{
			// Actor no longer detected, remove it and remove outline
			ITankInterface::Execute_OutlineTank(It->GetActor(), false);
			It.RemoveCurrent();
		}
	}

	// add any hit results that were previously not present
	for (const FHitResult& Element : CurrentHitResults)
		if (!HighlightedEnemyTanks.Contains(Element))
			HighlightedEnemyTanks.Add(Element);

	// highlight any and all actors that implement the interface
	for (const FHitResult& Hit : HighlightedEnemyTanks)
		if (Hit.IsValidBlockingHit())
			ITankInterface::Execute_OutlineTank(Hit.GetActor(), true);
}

void UTankHighlightingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TankCharacter)
		return;

	if (!TankCharacter->IsLocallyControlled())
		return;
	
	HighlightEnemyTanksIfDetected();
}
