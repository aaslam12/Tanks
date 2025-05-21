// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WB_HealthBar.generated.h"

class UTextBlock;
class UProgressBar;

USTRUCT(BlueprintType)
struct FHealthBarConfig
{
	GENERATED_BODY()

	/* Text for the Health Bar Text */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	FText Text;

	/* The progress bar style */ 
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Setup, meta=(AllowPrivateAccess="true"))
	FProgressBarStyle Style;
};

/**
 * @brief An abstract user widget that represents a health bar for the player, displaying both the health progress and associated text.
 *
 * This widget includes a health progress bar, text to indicate the player's status (alive or dead),
 * and methods to update the displayed health information. It provides BlueprintNativeEvents for
 * customization in Blueprint, allowing developers to alter or extend the default behavior.
 */
UCLASS(Abstract)
class TANKS_API UWB_HealthBar : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;

	// Used when the player is alive.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "/Setup", meta=(AllowPrivateAccess="true"))
	bool bIsDead;

	// Used when the player is alive.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "/Setup", meta=(AllowPrivateAccess="true"))
	FHealthBarConfig DefaultConfig;

	// Used when the player is dead.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "/Setup", meta=(AllowPrivateAccess="true"))
	FHealthBarConfig DeadConfig;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "/Components", meta=(AllowPrivateAccess="true", BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "/Components", meta=(AllowPrivateAccess="true", BindWidget))
	TObjectPtr<UTextBlock> HealthText;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ResetText() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetText(const FText& NewText) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetProgress(const float NewProgress) const;
	
	UFUNCTION(BlueprintNativeEvent)
	void PlayerIsDead();

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HealthChanged(const float NewHealth, const float MaxHealth);
};
