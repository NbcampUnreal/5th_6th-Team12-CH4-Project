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

	virtual void BeginPlay() override;

protected:

#pragma region Replication

	UFUNCTION()
	void OnRep_Items();

	UFUNCTION()
	void OnRep_CurrentSlotIndex();

	UFUNCTION(Server, Reliable)
	void Server_AddItem(FName ItemID);

	UFUNCTION(Server, Reliable)
	void Server_UseItem(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_ConsumeCurrentItem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion

public:

	UPROPERTY(ReplicatedUsing = OnRep_Items, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
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

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSlotIndex)
	int32 CurrentSlotIndex = INDEX_NONE;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ConsumeCurrentItem();
};
