// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WB_GunSight.generated.h"

class UOverlay;
class ATankController;
class UCanvasPanel;
class UImage;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TANKS_API UWB_GunSight : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(BlueprintReadWrite)
	FVector2D ScreenPosition;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<ATankController> TankController;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UOverlay> GunSightOverlay;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UImage> GunSightImage;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UImage> GunCenterImage;

	// Needs to run on tick.
	UFUNCTION(BlueprintCallable)
	void UpdateGunSightPosition() const;

	// Changes the gun sight color if player can shoot or not but runs on tick. migrate it to a better solution.
	UFUNCTION(BlueprintCallable)
	void SetTints() const;
};
