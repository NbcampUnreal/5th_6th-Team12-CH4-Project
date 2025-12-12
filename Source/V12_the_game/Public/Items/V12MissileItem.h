// Fill out your copyright notice in the Description page of Project Settings.
// V12MissileItem.h

#pragma once

#include "CoreMinimal.h"
#include "Items/V12ItemBase.h"
#include "V12MissileItem.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12MissileItem : public AV12ItemBase
{
	GENERATED_BODY()
	
public:
	AV12MissileItem()
	{
		ItemName = "Missile";
	}

	virtual void UseItem(AActor* TargetActor) override;
};
