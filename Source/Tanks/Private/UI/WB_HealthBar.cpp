// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WB_HealthBar.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UWB_HealthBar::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetDefaults();
}

void UWB_HealthBar::SetDefaults_Implementation()
{
	SetText(DefaultConfig.Text);
	SetProgress(1);
	HealthChanged(1,1);
}

void UWB_HealthBar::ResetText() const
{
	SetText(DefaultConfig.Text);
}

void UWB_HealthBar::SetText(const FText& NewText) const
{
	if (HealthText)
	{
		HealthText->SetText(NewText);
	}
}

void UWB_HealthBar::SetProgress(const float NewProgress) const
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

	SetVisibility(DeadConfig.bHideWidget ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
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

		SetVisibility(DefaultConfig.bHideWidget ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}
