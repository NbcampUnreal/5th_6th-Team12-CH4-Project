// Fill out your copyright notice in the Description page of Project Settings.
// V12MissileItem.h

#pragma once

#include "CoreMinimal.h"
#include "Items/V12ItemBase.h"
#include "V12MissileItem.generated.h"

class AV12HomingMissile;


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

	void SetTarget(AActor* InTarget);


	// 미사용 함수(자동 타겟 선택)
	AActor* SelectTarget(AActor* OwnerActor);

	AActor* FindBestTarget(AActor* OwnerActor);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AV12HomingMissile> MissileClass;

	UPROPERTY()
	AActor* MissileTargetActor;
};
