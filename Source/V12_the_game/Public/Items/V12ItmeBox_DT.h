// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "V12ItemsData.h"
#include "V12ItmeBox_DT.generated.h"

UCLASS()
class V12_THE_GAME_API AV12ItmeBox_DT : public AActor
{
	GENERATED_BODY()
	
public:	
	AV12ItmeBox_DT();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, Category = "Item Box")
	UDataTable* ItemDataTable;

	UFUNCTION()
	void OnOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 BodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:	
	virtual void Tick(float DeltaTime) override;

	TSubclassOf<AV12ItemBase> GetRandomItem();
};
