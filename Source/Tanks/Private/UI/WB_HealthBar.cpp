// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WB_HealthBar.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UWB_HealthBar::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetText(DefaultConfig.Text);
	SetProgress(1);
}

void UWB_HealthBar::ResetText_Implementation() const
{
	SetText(DefaultConfig.Text);
}

void UWB_HealthBar::SetText_Implementation(const FText& NewText) const
{
	if (HealthText)
	{
		HealthText->SetText(NewText);
	}
}

void UWB_HealthBar::SetProgress_Implementation(const float NewProgress) const
{
	if (HealthBar)
	{
		HealthBar->SetPercent(NewProgress);
	}
}

void UWB_HealthBar::PlayerIsDead_Implementation()
{
	SetText(DeadConfig.Text);
	SetProgress(0);
	bIsDead = true;
}

void UWB_HealthBar::HealthChanged_Implementation(const float NewHealth, const float MaxHealth)
{
	if (NewHealth <= 0)
	{
		PlayerIsDead();
	}
	else
	{
		SetText(DefaultConfig.Text);
		SetProgress(NewHealth / MaxHealth);
		bIsDead = false;
	}
}
