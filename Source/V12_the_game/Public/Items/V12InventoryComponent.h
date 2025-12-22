// Fill out your copyright notice in the Description page of Project Settings.
// V12InventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "V12InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);


USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Slot")
	FName RowName;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class V12_THE_GAME_API UV12InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UV12InventoryComponent();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_UseItem(int32 SlotIndex);

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Items;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 InventoryCapacity = 2;

	UPROPERTY(EditDefaultsONly, Category = "Inventory | UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory | UI")
	UUserWidget* InventoryWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory | Data")
	UDataTable* ItemsDataTable;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY()
	int32 CurrentSlotIndex = INDEX_NONE;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemID);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ConsumeCurrentItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(int32 SlotIndex);
};
