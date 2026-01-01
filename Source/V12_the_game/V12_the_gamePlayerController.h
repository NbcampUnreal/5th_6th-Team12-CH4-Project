// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Items/V12InventoryComponent.h"
#include "V12_the_gamePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UV12LockOnWidget;
class UV12LockOnMarker;
class AV12_the_gamePawn;
class UV12_the_gameUI;
class UV12_tachoMeter;


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

	/// 속도 타코미터 UI
	UPROPERTY(EditAnywhere, Category = "Vehicle|UI")
	TSubclassOf<UV12_tachoMeter> RaceUIClass;

	UPROPERTY()
	TObjectPtr<UV12_tachoMeter> RaceUI;
		
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

#pragma region UI
public:
	UFUNCTION(Client, Reliable)
	void CountdownCheck(const FText& nowCount);

	UFUNCTION(Client, Reliable)
	void BeginRace();

	UFUNCTION(Client, Reliable)
	void setRankMsg(int32 NewRank);

	UFUNCTION(Client, Reliable)
	void setLapMsg(int32 NewLap);

	UFUNCTION(Client, Reliable)
	void setFullLapMsg(int32 NewLap);

#pragma endregion

#pragma region Items
public:
	// Target Change
	UFUNCTION(BlueprintCallable)
	void CycleTarget();

	// LockOn Mode On
	void EnterLockOnMode();

	// Missile Fire
	UFUNCTION(BlueprintCallable)
	void ConfirmMissileFire();

	UFUNCTION(BlueprintCallable)
	void ScanTargets();

	UFUNCTION(Server, Reliable)
	void ServerSetLockedTarget(AActor* NewTarget);

	// LockOn Mode Off
	UFUNCTION(BlueprintCallable)
	void CancelLockOn();

	UFUNCTION(BlueprintCallable)
	void ChangeLockOnTarget();

	UFUNCTION(Server, Reliable)
	void Server_ScanTargets();

	UFUNCTION(Server, Reliable)
	void Server_CycleTarget();

	UFUNCTION(Server, Reliable)
	void Server_EnterLockOnMode();

	UFUNCTION(Server, Reliable)
	void Server_CancelLockOn();

	UFUNCTION(Server, Reliable)
	void Server_ConfirmMissileFire();

	UFUNCTION()
	void OnRep_LockedTarget();

	UFUNCTION()
	void OnRep_LockOnMode();

	// 락온모드 연속 진입 방지용 함수
	UFUNCTION(BlueprintPure)
	bool IsLockOnMode() const;

	void UpdateDefenseWidgets();
	void CreateDefenseWidget(AV12_the_gamePawn* CarPawn);
	void RemoveDefenseWidget(AV12_the_gamePawn* CarPawn);

	//Missile Defense Changed Event
	void HandleDefenseChanged(AV12_the_gamePawn* CarPawn);
	void BindAllPawnDefenseDelegates();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UV12InventoryComponent* InventoryComponent;

	// LockOn Target
	UPROPERTY(ReplicatedUsing = OnRep_LockedTarget, BlueprintReadOnly)
	AActor* LockedTarget = nullptr;

	UPROPERTY()
	AActor* CachedLockOnTarget;

	UPROPERTY()
	TSubclassOf<AActor> PendingMissileItemClass;

	UPROPERTY(ReplicatedUsing = OnRep_LockOnMode, BlueprintReadOnly)
	bool bIsLockOnMode = false;

protected:
	// 아이템창 UI 띄우기
	UPROPERTY(EditDefaultsOnly, Category = "UI|Inventory")
	TSubclassOf<UUserWidget> ItemHUDWidgetClass;

	// 아이템 띄우기
	UPROPERTY()
	UUserWidget* ItemWindowWidget;

	// Missile LockOn UI
	UPROPERTY(EditDefaultsOnly, Category = "UI|LockOn")
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	UPROPERTY()
	UV12LockOnWidget* LockOnWidget;

	// LockOnMarker(Indicator)
	// Missile LockOn UI
	UPROPERTY(EditDefaultsOnly, Category = "UI|LockOn")
	TSubclassOf<UUserWidget> LockOnMarkerClass;

	UPROPERTY()
	UV12LockOnMarker* LockOnMarker;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> LockOnCandidates;

	UPROPERTY(BlueprintReadOnly)
	UPrimitiveComponent* RootPrimitive;

	UPROPERTY(EditDefaultsOnly)
	float LockOnDotThreshold = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float MaxLockOnDistance = 9000.f;

	// Defense Item UI
	UPROPERTY(EditDefaultsONly, Category = "UI|DefenseItem")
	TSubclassOf<UUserWidget> DefenseWidgetClass;

	// Defense Item UI Map
	UPROPERTY()
	TMap<AV12_the_gamePawn*, UUserWidget*> DefenseWidgets;

	// 현재 타겟 인덱스
	int32 CurrentTargetIndex = -1;

#pragma endregion
};
