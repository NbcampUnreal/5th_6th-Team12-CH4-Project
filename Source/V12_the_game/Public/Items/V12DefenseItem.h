// Fill out your copyright notice in the Description page of Project Settings.
// V12DefenseItem.h

#pragma once

#include "CoreMinimal.h"
#include "Items/V12ItemBase.h"
#include "V12DefenseItem.generated.h"


UCLASS()
class V12_THE_GAME_API AV12DefenseItem : public AV12ItemBase
{
	GENERATED_BODY()

public:
	AV12DefenseItem()
	{
		ItemID = "MD";
		ItemName = "DefenseItem";
	}

	// 아이템 유지 시간
	UPROPERTY(EditDefaultsOnly, Category = "Defense")
	float DefenseDuration = 7.0f;

	virtual void UseItem(AActor * TargetActor) override;
	
private:
	void DisableDefense(class AV12_the_gamePawn* Pwan);
};
