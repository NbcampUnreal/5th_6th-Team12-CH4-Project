// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "V12ItemBase.h"
#include "V12ItemsData.generated.h"

USTRUCT(BlueprintType)
struct FV12ItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<AV12ItemBase> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Duration; // 0이면 즉시 사용

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Weight = 1; // 확률 가중치

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ItemIcon;

};