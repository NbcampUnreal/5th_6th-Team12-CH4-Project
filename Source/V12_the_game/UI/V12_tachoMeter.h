// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "V12_tachoMeter.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API UV12_tachoMeter : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* NeedleImage;

	UPROPERTY(meta = (BindWidget))
	UImage* BackgroundImage;

	UPROPERTY(EditAnywhere, Category = "Tachometer")
	float MinAngle = -135.f;

	UPROPERTY(EditAnywhere, Category = "Tachometer")
	float MaxAngle = 135.f;

public:
	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateRPM(float CurrentRPM);
};
