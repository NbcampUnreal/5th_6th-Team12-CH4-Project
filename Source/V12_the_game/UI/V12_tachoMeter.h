// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
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

	UPROPERTY(meta = (BindWidget))
	UImage* SpeedNeedleImage;

	UPROPERTY(meta = (BindWidget))
	UImage* SpeedBackgroundImage;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* NowScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Countdown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NowLap;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RapSlash;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FullLap;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NowRank;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NowGear;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MsgGear;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NowSpeed;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MsgSpeed;

	UPROPERTY(EditAnywhere, Category = "Tachometer")
	float MinAngle = -139.f;

	UPROPERTY(EditAnywhere, Category = "Tachometer")
	float MaxAngle = 139.f;

public:
	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateRPM(float CurrentRPM);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateSpeed(float CurrentSpeed);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateScore(int32 NewScore);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateCountdown(const FText& NewText);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateRank(int32 NewRank);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateLap(int32 NewLap);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateFullLap(int32 NewLap);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateGearMsg(int32 NewGear);

	UFUNCTION(BlueprintCallable, Category = "Tachometer")
	void UpdateSpeedMsg(float CurrentSpeed);
};
