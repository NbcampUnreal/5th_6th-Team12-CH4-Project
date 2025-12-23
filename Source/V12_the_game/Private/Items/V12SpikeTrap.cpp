// Fill out your copyright notice in the Description page of Project Settings.
// V12SpikeTrap.cpp

#include "Items/V12SpikeTrap.h"
#include "Net/UnrealNetwork.h"


AV12SpikeTrap::AV12SpikeTrap()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AV12SpikeTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AV12SpikeTrap::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	// 땅속에서 시작
	InitialLocation = GetActorLocation();
	InitialLocation.Z += StartZOffset;
	SetActorLocation(InitialLocation);	

	// 목표 위치 설정
	TargetLocation = GetActorLocation();
	TargetLocation.Z += RaiseHeight;

	// 0.01초마다 RaiseUpdate 호출
	GetWorld()->GetTimerManager().SetTimer(RaiseTimer, this, &AV12SpikeTrap::RaiseUpdate, 0.01f, true);
}

void AV12SpikeTrap::RaiseUpdate()
{
	if (!HasAuthority())
	{
		return;
	}

	ElapsedTime += 0.01f;

	float Alpha = FMath::Clamp(ElapsedTime / RaiseDuration, 0.f, 1.f);
	FVector NewLocation = FMath::Lerp(InitialLocation, TargetLocation, Alpha);
	SetActorLocation(NewLocation);

	if (Alpha >= 1.f)
	{
		// 상승 완료
		GetWorld()->GetTimerManager().ClearTimer(RaiseTimer);

		// 일정 시간 후 액터 제거
		GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AV12SpikeTrap::Destroy, LifeTimeAfterRaise, false);
	}
}

void AV12SpikeTrap::Destroy()
{
	Super::Destroy();
}

