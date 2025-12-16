// Fill out your copyright notice in the Description page of Project Settings.
// V12LockOnWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "V12LockOnWidget.generated.h"

class UVerticalBox;


UCLASS()
class V12_THE_GAME_API UV12LockOnWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void LockOnWidgetShow(bool bShow); 

	UFUNCTION(BlueprintCallable)
	void UpdateLockOnScreenPos(const FVector2D& ScreenPos);

protected:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalBox_LockOn;

};
