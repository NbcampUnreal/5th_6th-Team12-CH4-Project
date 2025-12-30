// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "V12_MainGameMode.generated.h"

class AV12_the_gamePlayerController;

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12_MainGameMode : public AGameMode
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Color")
	TArray<FLinearColor> PresetColors;

protected:
	virtual void BeginPlay() override;

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	TArray<AV12_the_gamePlayerController*> A_PC;

	FTimerHandle TestTimer;
	void TestFunc();

	FTimerHandle CountdownTimer;
	void StartCountdown();
	void CountdownFunc();

	int32 countdownCount;
	
};
