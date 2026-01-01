// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/V12_ARacingManager.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Engine/OverlapResult.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "V12_the_gamePawn.h"
#include "V12_the_gamePlayerController.h"

// Sets default values
AV12_ARacingManager::AV12_ARacingManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AV12_ARacingManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AV12_ARacingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;

	if (!TrackSplineComponent || RacerInfos.Num() == 0) return;

	UpdateRacerProgress();
	CalculateRankings();

}


void AV12_ARacingManager::RegisterRacer(AActor* NewRacer)
{
	if (NewRacer)
	{
		//FRacerInfo NewInfo;
		//NewInfo.RacerActor = NewRacer;
		//NewInfo.CurrentLap = 1; // 1랩부터 시작
		//RacerInfos.Add(NewInfo);
		
		FRacerInfo NewInfo;
		AV12_the_gamePawn* RacerPawn = Cast<AV12_the_gamePawn>(NewRacer);
		if (IsValid(RacerPawn))
		{
			NewInfo.RacerActor = RacerPawn;
		}
		NewInfo.CurrentLap = 1;

		// 실제 위치의 스플라인 거리
		NewInfo.CurrentDistanceOnSpline = TrackSplineComponent->GetDistanceAlongSplineAtLocation(
			NewRacer->GetActorLocation(),
			ESplineCoordinateSpace::World
		);

		NewInfo.TotalDistanceTraveled = 0.0f;
		RacerInfos.Add(NewInfo);
	}
}

void AV12_ARacingManager::SetTrackSpline(USplineComponent* SplineComp)
{
	TrackSplineComponent = SplineComp;
	if (TrackSplineComponent)
	{
		TrackLength = TrackSplineComponent->GetSplineLength();
		// 스플라인이 닫힌 루프가 아닐 수도 있으니 일단 true
		TrackSplineComponent->SetClosedLoop(true);
	}
}

void AV12_ARacingManager::raceStart()
{
	FindLongestSplineInRange(5000.0f);
	RegisterAllRacersInLevel();
}

void AV12_ARacingManager::UpdateRacerProgress()
{
	for (FRacerInfo& Info : RacerInfos)
	{
		if (Info.bFinished || !Info.RacerActor) continue;

		float NewDistance = TrackSplineComponent->GetDistanceAlongSplineAtLocation(
			Info.RacerActor->GetActorLocation(),
			ESplineCoordinateSpace::World
		);

		float DistanceDelta = NewDistance - Info.CurrentDistanceOnSpline;
		float Threshold = TrackLength * 0.7f; // 트랙을 70% 이상 이동했을 때 랩 변경 감지용

		// 정방향 랩 통과
		if (DistanceDelta < -Threshold)
		{
			Info.CurrentLap++;
		}
		// 역방향 랩 통과 (Distance: 100 -> 32000)
		else if (DistanceDelta > Threshold)
		{
			// 시작 지점에서 뒤로 가는 경우를 방지
			if (Info.CurrentLap > 1)
			{
				Info.CurrentLap--;
			}
			else
			{
				// 1랩때 후진으로 시작 지점 못넘게 하기
				NewDistance = 0.0f;
			}
		}

		Info.CurrentDistanceOnSpline = NewDistance;
		Info.TotalDistanceTraveled = ((Info.CurrentLap - 1) * TrackLength) + Info.CurrentDistanceOnSpline;

		if (Info.CurrentLap > MaxLaps)
		{
			Info.bFinished = true;
		}
	}
}

void AV12_ARacingManager::CalculateRankings()
{
	Algo::Sort(RacerInfos, [](const FRacerInfo& A, const FRacerInfo& B)
		{
			// A가 B보다 더 멀리 갔으면 true
			return A.TotalDistanceTraveled > B.TotalDistanceTraveled;
		});

	// 정렬 후 순위 할당
	for (int32 i = 0; i < RacerInfos.Num(); ++i)
	{
		RacerInfos[i].CurrentRank = i + 1;

		AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(RacerInfos[i].RacerActor->GetController());
		if (IsValid(PC))
		{
			PC->setLapMsg(RacerInfos[i].CurrentLap);
			PC->setRankMsg(RacerInfos[i].CurrentRank);
		}


		// 디버깅용 로그
		 //UE_LOG(LogTemp, Log, TEXT("Rank %d : %s (Lap: %d, Dist: %.2f)"), 
		 //   i + 1, 
		 //   *RacerInfos[i].RacerActor->GetName(), 
		 //   RacerInfos[i].CurrentLap, 
		 //   RacerInfos[i].TotalDistanceTraveled);
	}
}

void AV12_ARacingManager::RegisterAllRacersInLevel()
{
	TArray<AActor*> FoundActors;
	// 월드에 있는 모든 차량 액터 찾는 방법
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AV12_the_gamePawn::StaticClass(), FoundActors);
	UE_LOG(LogTemp, Error, TEXT("Found %d Racers in Level"), FoundActors.Num());
	for (AActor* Actor : FoundActors)
	{
		UE_LOG(LogTemp, Error, TEXT("Registering Racer: %s"), *Actor->GetName());
		AV12_the_gamePawn* CarPawn = Cast<AV12_the_gamePawn>(Actor);
		if (IsValid(CarPawn))
		{
			UE_LOG(LogTemp, Error, TEXT("Valid Racer Pawn: %s"), *CarPawn->GetName());
			RegisterRacer(CarPawn); // 아까 만든 등록 함수 호출

			/// default UI Setting
			AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(CarPawn->GetController());
			if (IsValid(PC))
			{
				PC->setFullLapMsg(MaxLaps);
				PC->setLapMsg(1);
				PC->setRankMsg(1);
			}			
		}
	}

	if(FoundActors.Num() > 0)
	{
		FVector CarLocation = FoundActors[0]->GetActorLocation();

		// 0번 차량을 스플라인 영점의 기준점으로 삼는다.
		float NewDistance = TrackSplineComponent->GetDistanceAlongSplineAtLocation(
			CarLocation,
			ESplineCoordinateSpace::World
		);

		raceDistanceDelay = NewDistance;
		
	}
}

AActor* AV12_ARacingManager::FindLongestSplineInRange(float SearchRadius)
{
	AActor* LongestSplineActor = nullptr;
	float MaxLength = -1.0f;
	USplineComponent* LongestSplineComp = nullptr;
	
	// 구체 영역 내의 액터들을 감지 (Overlap 사용)
	TArray<FOverlapResult> OverlapResults;
	FVector StartLocation = GetActorLocation();
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SearchRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		StartLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		CollisionShape,
		QueryParams
	);

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor)
			{
				// 스플라인 컴포넌트 찾기
				USplineComponent* SplineComp = HitActor->FindComponentByClass<USplineComponent>();
				// TrackSplineComponent = SplineComp;

				if (SplineComp)
				{
					// 여러개 감지될 수 있으니 제일 긴걸 트랙으로 삼기 위해 길이 비교
					float CurrentLength = SplineComp->GetSplineLength();
					UE_LOG(LogTemp, Warning,
						TEXT("Found Spline Actor: %s with Length: %.2f"),
						*HitActor->GetName(),
						CurrentLength
					);

					if (CurrentLength > MaxLength)
					{
						MaxLength = CurrentLength;
						// LongestSplineActor = Cast<AActor>(HitActor);
						LongestSplineActor = HitActor;
						LongestSplineComp = SplineComp;
					}
				}
			}
		}
		if (IsValid(LongestSplineComp))
		{
			SetTrackSpline(LongestSplineComp);
		}
	}

	return LongestSplineActor;
}
