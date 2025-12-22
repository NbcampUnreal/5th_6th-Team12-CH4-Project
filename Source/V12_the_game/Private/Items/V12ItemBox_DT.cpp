// Fill out your copyright notice in the Description page of Project Settings.
// V12ItemBox_DT.cpp

#include "Items/V12ItemBox_DT.h"
#include "Components/BoxComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "V12_the_gamePawn.h"
#include "TimerManager.h"
#include "Items/V12InventoryComponent.h"


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

	// 멀티플레이어 설정
	bReplicates = true;
	SetReplicateMovement(true);
}

void AV12ItemBox_DT::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AV12ItemBox_DT::BeginPlay()
{
	Super::BeginPlay();
}

void AV12ItemBox_DT::Multicast_SetActive_Implementation(bool bActive)
{
	BoxMesh->SetVisibility(bActive);
	CollisionBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AV12ItemBox_DT::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{ 
	// 서버만 처리
	if (!HasAuthority())
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(OtherActor->GetInstigatorController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Car Overlapped!"));

		FName RandomItemRow = GetRandomItem();
		if (RandomItemRow == NAME_None)
		{
			UE_LOG(LogTemp, Warning, TEXT("No item returned from table!"));
			return;
		}

		if (UV12InventoryComponent* InvComp = PC->FindComponentByClass<UV12InventoryComponent>())
		{
			InvComp->AddItem(RandomItemRow);
		}

		UE_LOG(LogTemp, Warning, TEXT("Player acquired item: %s"), *RandomItemRow.ToString());

		//BoxMesh->SetVisibility(false);
		Multicast_SetActive(false);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AV12ItemBox_DT::Respawn, RespawnTime, false);
	}
}

void AV12ItemBox_DT::Respawn()
{
	//BoxMesh->SetVisibility(true);
	Multicast_SetActive(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	UE_LOG(LogTemp, Warning, TEXT("ItemBox Respawned"));
}

FName AV12ItemBox_DT::GetRandomItem()
{
	if (!ItemDataTable) return NAME_None;

	TArray<FName> RowNames = ItemDataTable->GetRowNames();
	if (RowNames.Num() == 0) return NAME_None;

	int32 TotalWeight = 0;

	// 전체 확률 합산
	for (FName RowName : RowNames)
	{
		if (FV12ItemData* Row = ItemDataTable->FindRow<FV12ItemData>(RowName, ""))
		{
			TotalWeight += Row->Weight;
		}
	}

	if (TotalWeight <= 0) return NAME_None;

	int32 RandomValue = UKismetMathLibrary::RandomIntegerInRange(1, TotalWeight);

	int32 AccumulatedWeight = 0;
	for (FName RowName : RowNames)
	{
		if (FV12ItemData* Row = ItemDataTable->FindRow<FV12ItemData>(RowName, ""))
		{
			AccumulatedWeight += Row->Weight;
			if (RandomValue <= AccumulatedWeight)
			{
				return RowName;
			}
		}
	}
	return NAME_None;
}