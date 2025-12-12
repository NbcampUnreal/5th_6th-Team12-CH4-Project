// Fill out your copyright notice in the Description page of Project Settings.
// V12InventoryComponent.cpp

#include "Items/V12InventoryComponent.h"
#include "Blueprint/UserWidget.h"

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
		if (Items[i].ItemID == NAME_None)
		{
			Items[i].ItemID = ItemID;

			if (GEngine)
			{
				FString const Msg = FString::Printf(TEXT("아이템 저장! Slot %d: %s"), i, *ItemID.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
			}

			return;
		}
	}

	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("인벤토리가 가득 찼습니다!"));
	}
}
