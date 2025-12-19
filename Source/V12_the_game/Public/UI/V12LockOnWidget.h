// Fill out your copyright notice in the Description page of Project Settings.
// V12LockOnWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "V12LockOnWidget.generated.h"

class UV12LockOnMarker;


UCLASS()
class V12_THE_GAME_API UV12LockOnWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* LockOnButtons;

	UFUNCTION(BlueprintCallable)
	void ShowLockOn();

	UFUNCTION(BlueprintCallable)
	void HideLockOn();
};
