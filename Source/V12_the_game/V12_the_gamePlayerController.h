// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Items/V12InventoryComponent.h"
#include "V12_the_gamePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UV12LockOnWidget;
class AV12_the_gamePawn;
class UV12_the_gameUI;


/**
 *  Vehicle Player Controller class
 *  Handles input mapping and user interface
 */
UCLASS(abstract, Config="Game")
class AV12_the_gamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AV12_the_gamePlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** If true, the optional steering wheel input mapping context will be registered */
	UPROPERTY(EditAnywhere, Category = "Input|Steering Wheel Controls")
	bool bUseSteeringWheelControls = false;

	/** Optional Input Mapping Context to be used for steering wheel input.
	 *  This is added alongside the default Input Mapping Context and does not block other forms of input.
	 */
	UPROPERTY(EditAnywhere, Category = "Input|Steering Wheel Controls", meta = (EditCondition = "bUseSteeringWheelControls"))
	UInputMappingContext* SteeringWheelInputMappingContext;

	/** Type of vehicle to automatically respawn when it's destroyed */
	UPROPERTY(EditAnywhere, Category="Vehicle|Respawn")
	TSubclassOf<AV12_the_gamePawn> VehiclePawnClass;

	/** Pointer to the controlled vehicle pawn */
	TObjectPtr<AV12_the_gamePawn> VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, Category="Vehicle|UI")
	TSubclassOf<UV12_the_gameUI> VehicleUIClass;

	/** Pointer to the UI widget */
	UPROPERTY()
	TObjectPtr<UV12_the_gameUI> VehicleUI;
		
protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input setup */
	virtual void SetupInputComponent() override;

public:

	/** Update vehicle UI on tick */
	virtual void Tick(float Delta) override;

protected:

	/** Pawn setup */
	virtual void OnPossess(APawn* InPawn) override;

	/** Handles pawn destruction and respawning */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedPawn);

#pragma region Items

private:
	// 아이템창 UI 띄우기
	UPROPERTY(EditDefaultsOnly, Category = "UI|Inventory")
	TSubclassOf<UUserWidget> ItemHUDWidgetClass;

	UPROPERTY()
	UUserWidget* ItemWindowWidget;

	// Missile LockOn UI
	UPROPERTY(EditDefaultsOnly, Category = "UI|LockOn")
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	UPROPERTY()
	UV12LockOnWidget* LockOnWidget;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UV12InventoryComponent* InventoryComponent;

	// LockOn Target
	UPROPERTY(BlueprintReadOnly)
	AActor* LockedTarget = nullptr;

	UPROPERTY()
	TSubclassOf<AActor> PendingMissileItemClass;

	// Target Change
	UFUNCTION(BlueprintCallable)
	void CycleTarget();

	// LockOn Mode On
	void EnterLockOnMode();

	// LockOn Mode Off
	UFUNCTION(BlueprintCallable)
	void ExitLockOnMode();

	UFUNCTION(BlueprintCallable)
	void ConfirmMissileFire();

	UFUNCTION(BlueprintCallable)
	void ScanTargets();

	UFUNCTION(Server, Reliable)
	void ServerSetLockedTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable)
	void CancelLockOn();

protected:

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> LockOnCandidates;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLockOnMode = false;

	UPROPERTY(EditDefaultsOnly)
	float LockOnDotThreshold = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float MaxLockOnDistance = 5000.f;

#pragma endregion

};
