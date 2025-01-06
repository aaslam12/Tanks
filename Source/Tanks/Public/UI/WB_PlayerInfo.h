// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WB_PlayerInfo.generated.h"

class UTextBlock;
class UBorder;
class UImage;
class UOverlay;
class UCanvasPanel;
/**
 * 
 */
UCLASS(Blueprintable)
class TANKS_API UWB_PlayerInfo : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> Main;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UImage> Line;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> Placque_Overlay;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UBorder> Border;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UImage> PlacqueImage;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> TextBlock;

public:
	UFUNCTION(BlueprintCallable)
	void UpdateScreenPosition(const FVector2D& ScreenPosition);
};
