// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "V12_ARacingManager.generated.h"

class AV12_the_gamePawn;

USTRUCT(BlueprintType)
struct FRacerInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    AV12_the_gamePawn* RacerActor = nullptr; // 차량 액터

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentLap = 0; // 현재 바퀴 수 (0부터 시작 or 1부터 시작)

    UPROPERTY(BlueprintReadOnly)
    float CurrentDistanceOnSpline = 0.0f; // 이번 바퀴에서의 진행 거리

    UPROPERTY(BlueprintReadOnly)
    float TotalDistanceTraveled = 0.0f; // 랭킹 산정용 절대 거리

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentRank = 0; // 현재 순위

    UPROPERTY(BlueprintReadOnly)
    bool bFinished = false; // 레이스 완주 여부
};

UCLASS()
class V12_THE_GAME_API AV12_ARacingManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AV12_ARacingManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // 아까 찾으신 스플라인 컴포넌트를 여기에 저장한다고 가정합니다.
    UPROPERTY()
    USplineComponent* TrackSplineComponent;

    // 경주에 참여하는 차량들의 정보 목록
    UPROPERTY(BlueprintReadOnly)
    TArray<FRacerInfo> RacerInfos;

    // 트랙 전체 길이 (캐싱용)
    float TrackLength;

    // 목표 랩 수
    const int32 MaxLaps = 3;

public:
    // 차량 등록 함수 (게임 시작 시 호출)
    void RegisterRacer(AActor* NewRacer);

    // 스플라인 설정 함수 (질문자님의 코드로 찾은 뒤 이 함수 호출)
    void SetTrackSpline(USplineComponent* SplineComp);

    void raceStart();

private:
    void UpdateRacerProgress();
    void CalculateRankings();
    void RegisterAllRacersInLevel();


    AActor* FindLongestSplineInRange(float SearchRadius);

    float raceDistanceDelay = 0.f;
};
