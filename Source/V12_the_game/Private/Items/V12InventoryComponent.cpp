// Fill out your copyright notice in the Description page of Project Settings.
// V12InventoryComponent.cpp

#include "Items/V12InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Items/V12SpikeItem.h"
#include "Items/V12SpeedBoostItem.h"
#include "Items/V12ItemsData.h"
#include "Items/V12MissileItem.h"
#include "Items/V12DefenseItem.h"
#include "V12_the_gamePlayerController.h"

class UV12ItemBase;

UV12InventoryComponent::UV12InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UV12InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Items.SetNum(InventoryCapacity);

	if (InventoryWidgetClass)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());

		if (PlayerController)
		{
			InventoryWidget = CreateWidget<UUserWidget>(PlayerController, InventoryWidgetClass);
		}
	}
}

void UV12InventoryComponent::AddItem(FName ItemID)
{
	if (ItemID == NAME_None) 
	{
		return;
	}

	for(int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].RowName == NAME_None)
		{
			Items[i].RowName = ItemID;


			OnInventoryUpdated.Broadcast();

			if (GEngine)
			{
				FString const Msg = FString::Printf(TEXT("아이템 저장! 슬롯 %d : %s"), i + 1, *ItemID.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
			}

			return;
		}
	}

	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("슬롯이 가득 찼습니다!"));
	}
}

void UV12InventoryComponent::ConsumeCurrentItem()
{
	if (!Items.IsValidIndex(CurrentSlotIndex))
	{
		return;
	}

	Items[CurrentSlotIndex].RowName = NAME_None;
	CurrentSlotIndex = INDEX_NONE;
	OnInventoryUpdated.Broadcast();
}


void UV12InventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Items.IsValidIndex(SlotIndex)) 
	{	
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("사용할 아이템이 없습니다!")); 

		return; 
	}

	CurrentSlotIndex = SlotIndex;

	FName ItemIDToUse = Items[SlotIndex].RowName;
	if(ItemIDToUse == NAME_None)
	{
		return;
	}

	// 서버(권한) 체크 : 서버에서만 아이템 사용 처리
	//if (!GetOwner() || !GetOwner()->HasAuthority())
	//{
	//	return;
	//}

	// OwnerActor 얻기 (인벤토리가 어디에 붙어있는지에 따라 안전하게 처리)
	AActor* OwnerActor = GetOwner();
	APawn* OwnerPawn = nullptr;
	APlayerController* OwnerPC = nullptr;

	// Owner가 Pawn일 경우
	OwnerPawn = Cast<APawn>(OwnerActor);

	// Owner가 PlayerController (인벤토리를 컨트롤러에 붙였다면) 일 경우 Pawn을 얻음
	if (!OwnerPawn)
	{
		OwnerPC = Cast<APlayerController>(OwnerActor);
		if (OwnerPC)
		{
			OwnerPawn = OwnerPC->GetPawn();
		}
	}

	AActor* TargetActor = OwnerPawn ? static_cast<AActor*>(OwnerPawn) : OwnerActor;
	if (!TargetActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}


	if(ItemIDToUse == "SB")
	{
		if (!ItemsDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemsDataTable is NULL"));
			return;
		}

		const FV12ItemData* ItemData =
			ItemsDataTable->FindRow<FV12ItemData>(FName("SB"), TEXT("UseItem SB"));

		if (!ItemData || !ItemData->ItemClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Speed Boost Item Row or ItemClass is NULL"));
			return;
		}

		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (PC)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.F, FColor::Yellow, TEXT("스피드 부스트 아이템 사용!"));
			}

			AV12SpeedBoostItem* SpeedBoostItem =
				NewObject<AV12SpeedBoostItem>(this, ItemData->ItemClass);

			SpeedBoostItem->UseItem(TargetActor);

			ConsumeCurrentItem();
		}
	}
	else if (ItemIDToUse == "ST")
	{
		if (!ItemsDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemsDataTable is NULL"));
			return;
		}

		const FV12ItemData* ItemData =
			ItemsDataTable->FindRow<FV12ItemData>(FName("ST"), TEXT("UseItem ST"));


		if (!ItemData || !ItemData->ItemClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Spike Item Row or ItemClass is NULL"));
			return;
		}
	
		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (PC)
		{
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.F, FColor::Yellow, TEXT("스파이크 트랩 아이템 사용!"));
			}

			AV12SpikeItem* SpikeItem =
				NewObject<AV12SpikeItem>(this, ItemData->ItemClass);

			SpikeItem->UseItem(TargetActor);
	
			ConsumeCurrentItem();
		}
	}
	else if (ItemIDToUse == "HM")
	{
		if (!ItemsDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemsDataTable is NULL"));
			return;
		}

		const FV12ItemData* ItemData =
			ItemsDataTable->FindRow<FV12ItemData>(FName("HM"), TEXT("UseItem HM"));

		if (!ItemData || !ItemData->ItemClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Homing Missile Item Row or ItemClass is NULL"));
			return;
		}

		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (!PC)
		{
			return;
		}

		if (AV12_the_gamePlayerController* V12PC = Cast<AV12_the_gamePlayerController>(PC))
		{
			if (V12PC->IsLockOnMode())
			{
				UE_LOG(LogTemp, Log, TEXT("Already in LockOnMode - Skip"));
				return;
			}

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("유도 미사일 락온 모드 진입"));
			}


			// 미사일아이템 클래스 전달
			V12PC->PendingMissileItemClass = ItemData->ItemClass;

			// 발사하지 않고 락온 모드만 진입
			V12PC->EnterLockOnMode();
		}
	}
	else if (ItemIDToUse == "MD")
	{
		if (!ItemsDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("ItemsDataTable is NULL"));
			return;
		}

		const FV12ItemData* ItemData =
			ItemsDataTable->FindRow<FV12ItemData>(FName("MD"), TEXT("UseItem MD"));

		if (!ItemData || !ItemData->ItemClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Spike Item Row or ItemClass is NULL"));
			return;
		}

		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (PC)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.F, FColor::Yellow, TEXT("미사일 방어 아이템 사용!"));
			}

			AV12DefenseItem* DefenseItem =
				NewObject<AV12DefenseItem>(this, ItemData->ItemClass);

			DefenseItem->UseItem(TargetActor);

			ConsumeCurrentItem();
		}
	}

	OnInventoryUpdated.Broadcast();
}
