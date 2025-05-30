// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WB_GunSight.h"

#include "TankController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"

void UWB_GunSight::NativePreConstruct()
{
	Super::NativePreConstruct();

	// GunSight
	GunSightOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

	// GunCenter
	GunCenterImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UWB_GunSight::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetOwningPlayer())
		if (auto e = Cast<ATankController>(GetOwningPlayer()))
			TankController = e;
}

void UWB_GunSight::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateGunSightPosition();
	SetTints();
}

void UWB_GunSight::UpdateGunSightPosition() const
{
	UWidgetLayoutLibrary::SlotAsCanvasSlot(GunSightOverlay)->SetPosition(ScreenPosition);
}

void UWB_GunSight::SetTints() const
{
	if (TankController.IsValid() && GunSightOverlay && GunCenterImage && GunSightImage)
	{
		GunSightImage->SetBrushTintColor(TankController->CanShoot() ? FLinearColor::White : FLinearColor::Red);
		GunCenterImage->SetBrushTintColor(TankController->CanShoot() ? FLinearColor::White : FLinearColor::Red);
	}
}
