// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WB_PlayerInfo.h"

#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"

void UWB_PlayerInfo::NativeConstruct()
{
	Super::NativeConstruct();

	CanvasPanel.Get()->AddChildToCanvas(Main);

	Main->AddChildToOverlay(Line);
	Main->AddChildToOverlay(Placque_Overlay);

	Cast<UCanvasPanelSlot>(Main->Slot)->SetAlignment(FVector2D(0, 0.35));
	Cast<UCanvasPanelSlot>(Main->Slot)->SetAutoSize(true);

	Cast<UOverlaySlot>(Line->Slot)->SetHorizontalAlignment(HAlign_Center);
	Cast<UOverlaySlot>(Line->Slot)->SetVerticalAlignment(VAlign_Fill);

	Placque_Overlay->AddChildToOverlay(Border);
	Placque_Overlay->AddChildToOverlay(TextBlock);

	Cast<UOverlaySlot>(Border->Slot)->SetHorizontalAlignment(HAlign_Fill);
	Cast<UOverlaySlot>(Border->Slot)->SetVerticalAlignment(VAlign_Top);

	Border->SetHorizontalAlignment(HAlign_Fill);
	Border->SetVerticalAlignment(VAlign_Top);
	Border->SetBrushColor(FLinearColor::Black);

	Cast<UOverlaySlot>(TextBlock->Slot)->SetHorizontalAlignment(HAlign_Fill);
	Cast<UOverlaySlot>(TextBlock->Slot)->SetVerticalAlignment(VAlign_Center);

	TextBlock->SetText(FText::FromString("Player Name Here"));

	TextBlock->SetTextTransformPolicy(ETextTransformPolicy::ToUpper);
	TextBlock->SetJustification(ETextJustify::Type::Center);

	Border->AddChild(PlacqueImage);

	Cast<UBorderSlot>(PlacqueImage->Slot)->SetPadding(FMargin(5));
	Cast<UBorderSlot>(PlacqueImage->Slot)->SetVerticalAlignment(VAlign_Top);
}

void UWB_PlayerInfo::UpdateScreenPosition(const FVector2D& ScreenPosition)
{
	if (Main)
		if (Cast<UCanvasPanelSlot>(Main->Slot))
			Cast<UCanvasPanelSlot>(Main->Slot)->SetPosition(ScreenPosition);
}
