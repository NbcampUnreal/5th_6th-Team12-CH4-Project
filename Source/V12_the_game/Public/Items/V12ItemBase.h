// Fill out your copyright notice in the Description page of Project Settings.
// V12ItemBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "V12ItemBase.generated.h"

UCLASS()
class V12_THE_GAME_API AV12ItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AV12ItemBase();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	FString ItemName;

	UPROPERTY(EditDefaultsONly, Category = "Item")
	UTexture2D* ItemIcon;

public:	
	UFUNCTION(BlueprintCallable)
	virtual void UseItem(class AActor* TargetActor);

	FORCEINLINE FString GetItemName() const { return ItemName; }
	FORCEINLINE UTexture2D* GetItemIcon() const { return ItemIcon; }
};
