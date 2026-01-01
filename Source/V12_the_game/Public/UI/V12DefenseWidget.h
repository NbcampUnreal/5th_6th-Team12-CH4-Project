// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "V12_the_gamePawn.h"
#include "V12DefenseWidget.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API UV12DefenseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetTargetPawn(AV12_the_gamePawn* InPawn);
	
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	APawn* TargetPawn = nullptr;
};
