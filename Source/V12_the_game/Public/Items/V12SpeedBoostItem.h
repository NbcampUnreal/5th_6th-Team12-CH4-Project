// Fill out your copyright notice in the Description page of Project Settings.
// V12SpeedBoostItem.h

#pragma once

#include "CoreMinimal.h"
#include "Items/V12ItemBase.h"
#include "V12SpeedBoostItem.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12SpeedBoostItem : public AV12ItemBase
{
	GENERATED_BODY()
	
public:
	AV12SpeedBoostItem()
	{
		ItemName = "SpeedBoost";
	}

	virtual void UseItem(AActor* TargetActor) override;
};
