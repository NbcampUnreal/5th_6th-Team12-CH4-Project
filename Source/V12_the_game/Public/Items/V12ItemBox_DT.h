// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "V12ItemsData.h"
#include "V12InventoryComponent.h"
#include "V12ItemBox_DT.generated.h"

class URotatingMovementComponent;
class UStaticMeshComponent;

UCLASS()
class V12_THE_GAME_API AV12ItemBox_DT : public AActor
{
	GENERATED_BODY()

public:
	AV12ItemBox_DT();

protected:
	virtual void BeginPlay() override;

	void Respawn();

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	URotatingMovementComponent* RotatingComp;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BoxMesh;

	UPROPERTY(EditAnywhere, Category = "Item Box")
	float RespawnTime = 10.f;

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

	FTimerHandle RespawnTimer;
	FName GetRandomItem();

public:
	virtual void Tick(float DeltaTime) override;

};
