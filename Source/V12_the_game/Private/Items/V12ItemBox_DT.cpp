// Fill out your copyright notice in the Description page of Project Settings.
// V12ItemBox_DT.cpp

#include "Items/V12ItemBox_DT.h"
#include "Components/BoxComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "V12_the_gameSportsCar.h"
#include "TimerManager.h"

AV12ItemBox_DT::AV12ItemBox_DT()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AV12ItemBox_DT::OnOverlap);

	// 박스 메시
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	BoxMesh->SetupAttachment(RootComponent);
	BoxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 회전 컴포넌트
	RotatingComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComp"));
	RotatingComp->RotationRate = FRotator(0.f, 120.f, 0.f); // 초당 120도 회전
}

void AV12ItemBox_DT::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AV12ItemBox_DT::BeginPlay()
{
	Super::BeginPlay();
}


void AV12ItemBox_DT::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AV12_the_gameSportsCar* Car = Cast<AV12_the_gameSportsCar>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Car Overlapped!"));

		TSubclassOf<AV12ItemBase> Item = GetRandomItem();
		if (Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("Item Generated!"));
			Car->SetItem(Item);
			UE_LOG(LogTemp, Warning, TEXT("Player acquired item from DataTable!"));
		}

		BoxMesh->SetVisibility(false);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AV12ItemBox_DT::Respawn, RespawnTime, false);
	}
}

void AV12ItemBox_DT::Respawn()
{
	BoxMesh->SetVisibility(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	UE_LOG(LogTemp, Warning, TEXT("ItemBox Respawned"));
}

TSubclassOf<AV12ItemBase> AV12ItemBox_DT::GetRandomItem()
{
	if (!ItemDataTable) return nullptr;

	TArray<FName> RowNames = ItemDataTable->GetRowNames();

	int32 TotalWeigth = 0;

	// 전체 확률 합산
	for (FName RowName : RowNames)
	{
		if (FV12ItemData* Row = ItemDataTable->FindRow<FV12ItemData>(RowName, ""))
		{
			TotalWeigth += Row->Weight;
		}
	}

	int32 RandomValue = UKismetMathLibrary::RandomIntegerInRange(1, TotalWeigth);
	int32 AccumulatedWeight = 0;

	for (FName RowName : RowNames)
	{
		if (FV12ItemData* Row = ItemDataTable->FindRow<FV12ItemData>(RowName, ""))
		{
			AccumulatedWeight += Row->Weight;
			if (RandomValue <= AccumulatedWeight)
			{
				return Row->ItemClass;
			}
		}
	}

	return nullptr;
}



