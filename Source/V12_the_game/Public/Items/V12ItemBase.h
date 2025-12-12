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
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	FString ItemName = "BaseItem";

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	UTexture2D* ItemIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	float Duration = 0.f; // 0이면 즉시 사용


public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void UseItem(class AActor* TargetActor);

	FORCEINLINE FString GetItmeName() const
	{
		return ItemName;
	}
	
	FORCEINLINE UTexture2D* GetItemIcon() const
	{
		return ItemIcon;
	}

};
