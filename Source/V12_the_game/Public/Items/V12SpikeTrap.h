// Fill out your copyright notice in the Description page of Project Settings.
// V12SpikeTrap.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "V12SpikeTrap.generated.h"


UCLASS()
class V12_THE_GAME_API AV12SpikeTrap : public AActor
{
	GENERATED_BODY()
	
public:	
	AV12SpikeTrap();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Trap")
	float StartZOffset = -80.f; // 처음 땅속 스폰 위치

	UPROPERTY(EditAnywhere, Category = "Trap")
	float RaiseHeight = 80.f; // 올라올 높이

	UPROPERTY(EditAnywhere, Category = "Trap")
	float RaiseDuration = 0.25f; // 상승 시간(0.25초)

	UPROPERTY(EditAnywhere, Category = "Trap")
	float LifeTimeAfterRaise = 5.0f; // 상승 후 유지 시간

	float ElapsedTime = 0.f;
	FVector InitialLocation;
	FVector TargetLocation;

	FTimerHandle RaiseTimer;
	FTimerHandle DestroyTimer;

	void RaiseUpdate(); // 타이머로 상승 업데이트
	void Destroy(); // 타이머로 액터 제거
};
