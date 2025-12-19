// Fill out your copyright notice in the Description page of Project Settings.
// V12LockOnMarker.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "V12LockOnMarker.generated.h"

class UVerticalBox;


UCLASS()
class V12_THE_GAME_API UV12LockOnMarker : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	UPrimitiveComponent* TargetedComponent;

	UFUNCTION(BlueprintCallable)
	void SetMarkerVisible(bool bVisible);

	UFUNCTION(BlueprintCallable)
	void UpdateScreenPosition(const FVector2D& ScreenPos);
};
