// Fill out your copyright notice in the Description page of Project Settings.
// V12ItmeBox_DT.cpp

#include "Items/V12ItmeBox_DT.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "V12_the_gameSportsCar.h"

AV12ItmeBox_DT::AV12ItmeBox_DT()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AV12ItmeBox_DT::OnOverlap);	
}

void AV12ItmeBox_DT::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AV12ItmeBox_DT::BeginPlay()
{
	Super::BeginPlay();
}


void AV12ItmeBox_DT::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
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

		Destroy();
	}
}


TSubclassOf<AV12ItemBase> AV12ItmeBox_DT::GetRandomItem()
{
	if (!ItemDataTable) return nullptr;

	TArray<FName> RowNames = ItemDataTable->GetRowNames();

	int32 TotalWeigth = 0;

	// 전체 확률 합산
	for(FName RowName : RowNames)
	{
		if (FV12ItemData* Row = ItemDataTable->FindRow<FV12ItemData>(RowName, ""))
		{
			TotalWeigth += Row->Weight;
		}
	}

	int32 RandomValue = UKismetMathLibrary::RandomIntegerInRange(1, TotalWeigth);
	int32 AccumulatedWeight = 0;

	for(FName RowName : RowNames)
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

