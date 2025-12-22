// Fill out your copyright notice in the Description page of Project Settings.
// V12SpikeItem.h

#pragma once

#include "CoreMinimal.h"
#include "Items/V12ItemBase.h"
#include "Items/V12SpikeTrap.h"
#include "V12SpikeItem.generated.h"


UCLASS()
class V12_THE_GAME_API AV12SpikeItem : public AV12ItemBase
{
	GENERATED_BODY()
	
	AV12SpikeItem()
	{
		ItemID = "ST";
		ItemName = "Spike Trap";
	}

public:

	virtual void UseItem(AActor* TargetActor) override;

	UPROPERTY(EditDefaultsOnly, Category = "Spike")
	TSubclassOf<AV12SpikeTrap> SpikeTrapClass;

protected:
	UFUNCTION(Server, Reliable)
	void ServerSpawnSpikeTrap(AActor* TargetActor);
};
