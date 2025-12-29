// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "V12_MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12_MainGameMode : public AGameMode
{
	GENERATED_BODY()
protected:
	//로비 게임 모드로 이동 가능
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Color")
	TArray<FLinearColor> PresetColors;
};
