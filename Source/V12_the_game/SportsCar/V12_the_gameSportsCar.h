// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "V12_the_gamePawn.h"
#include "Items/V12ItemBase.h"
#include "V12_the_gameSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class AV12_the_gameSportsCar : public AV12_the_gamePawn
{
	GENERATED_BODY()
	
public:

	AV12_the_gameSportsCar();

	UPROPERTY(VisibleAnywhere)
	TSubclassOf<AV12ItemBase> CurrentItem;

	void SetItem(TSubclassOf<AV12ItemBase> NewItem);

	void UseItem();

};
