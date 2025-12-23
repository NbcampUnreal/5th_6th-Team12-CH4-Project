// Copyright Epic Games, Inc. All Rights Reserved.


#include "V12_the_gamePlayerController.h"
#include "EngineUtils.h"
#include "V12_the_gamePawn.h"
#include "V12_the_gameUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Blueprint/UserWidget.h"
#include "V12_the_game.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UI/V12LockOnWidget.h"
#include "UI/V12LockOnMarker.h"
#include "Items/V12MissileItem.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/V12_tachoMeter.h"


AV12_the_gamePlayerController::AV12_the_gamePlayerController()
{
	InventoryComponent = CreateDefaultSubobject<UV12InventoryComponent>(TEXT("InventoryComponent"));
}

void AV12_the_gamePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// ensure we're attached to the vehicle pawn so that World Partition streaming works correctly
	bAttachToPawn = true;

	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		VehicleUI = CreateWidget<UV12_the_gameUI>(this, VehicleUIClass);

		if (VehicleUI)
		{
			VehicleUI->AddToViewport();

		} else {

			UE_LOG(LogV12_the_game, Error, TEXT("Could not spawn vehicle UI widget."));

		}
	}

	// Inventory Widget Create
	if (ItemHUDWidgetClass)
	{
		if (APlayerController* LocalController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			ItemWindowWidget = CreateWidget<UUserWidget>(LocalController, ItemHUDWidgetClass);

			ItemWindowWidget->AddToViewport();
		}
	}

	// LockOn Widget Create
	if (LockOnWidgetClass)
	{
		LockOnWidget = CreateWidget<UV12LockOnWidget>(this, LockOnWidgetClass);
		if (LockOnWidget)
		{
			LockOnWidget->AddToViewport(50); // HUD보다 ??
			LockOnWidget->HideLockOn();
		}
	
	}

	if (IsLocalPlayerController())
	{
		SpeedUI = CreateWidget<UV12_tachoMeter>(this, SpeedUIClass);

		if (SpeedUI)
		{
			SpeedUI->AddToViewport();

		}
		else 
		{
			UE_LOG(LogV12_the_game, Error, TEXT("Could not spawn speed UI widget."));

		}
	}
}

void AV12_the_gamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AV12_the_gamePlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	/// RPM Update
	if (IsValid(VehiclePawn) && IsValid(SpeedUI) && IsValid(VehiclePawn))
	{
		float nowRPM = VehiclePawn->ChaosVehicleMovement->GetEngineRotationSpeed();
		SpeedUI->UpdateRPM(nowRPM);
		// UE_LOG(LogTemp, Warning, TEXT("Current RPM: %f"), nowRPM);
	}

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}

	// LockOn Wiget Position Update
	if (bIsLockOnMode && LockedTarget && LockOnWidget)
	{
		// LockOn Marker Posistion
		FVector2D ScreenPos;
		ProjectWorldLocationToScreen(
			LockedTarget->GetActorLocation() + FVector(0, 0, 100.f),
			ScreenPos, true
		);

		LockOnMarker->UpdateScreenPosition(ScreenPos);
	}

	// LockOn Distance Cancel

	// ?�온 중이 ?�니�??�무 것도 ????
	if (LockedTarget)
	{
		APawn* MyPawn = GetPawn();
		if (!MyPawn)
		{
			CancelLockOn();
			return;
		}

		const float Distance = FVector::Dist(
			LockedTarget->GetActorLocation(),
			MyPawn->GetActorLocation()
		);

		if (Distance > MaxLockOnDistance)
		{
			CancelLockOn();
		}
	}

}

void AV12_the_gamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<AV12_the_gamePawn>(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	VehiclePawn->OnDestroyed.AddDynamic(this, &AV12_the_gamePlayerController::OnPawnDestroyed);
}

void AV12_the_gamePlayerController::OnPawnDestroyed(AActor* DestroyedPawn)
{
	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// spawn a vehicle at the player start
		const FTransform SpawnTransform = ActorList[0]->GetActorTransform();

		if (AV12_the_gamePawn* RespawnedVehicle = GetWorld()->SpawnActor<AV12_the_gamePawn>(VehiclePawnClass, SpawnTransform))
		{
			// possess the vehicle
			Possess(RespawnedVehicle);
		}
	}
}

#pragma region Item(Missile)

void AV12_the_gamePlayerController::ScanTargets()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	LockOnCandidates.Empty();

	const FVector OwnerLoc = ControlledPawn->GetActorLocation();
	const FVector Forward = ControlledPawn->GetActorForwardVector();

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* TargetPawn = *It;
		if (!TargetPawn || TargetPawn == ControlledPawn) continue;

		FVector ToTarget = (TargetPawn->GetActorLocation() - OwnerLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(Forward, ToTarget);

		if (Dot >= LockOnDotThreshold)
		{
			LockOnCandidates.Add(TargetPawn);
		}
	}
}

void AV12_the_gamePlayerController::CycleTarget()
{
	ScanTargets();

	if (LockOnCandidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No LockOn Candidates"));

		ServerSetLockedTarget(nullptr);

		return;
	}

	CurrentTargetIndex =
		(CurrentTargetIndex + 1) % LockOnCandidates.Num();

	UE_LOG(LogTemp, Warning, TEXT("Locked Target: %s"), *LockOnCandidates[CurrentTargetIndex]->GetName());

	ServerSetLockedTarget(LockOnCandidates[CurrentTargetIndex]);
}

void AV12_the_gamePlayerController::ServerSetLockedTarget_Implementation(AActor* NewTarget)
{
	LockedTarget = NewTarget;

	if (LockOnWidget)
	{
		if (LockedTarget)
		{
			LockOnWidget->ShowLockOn();
		}
		else
		{
			LockOnWidget->HideLockOn();
		}
	}
}

void AV12_the_gamePlayerController::EnterLockOnMode()
{
	bIsLockOnMode = true;

	ScanTargets();
	CycleTarget();

	if (LockOnWidget)
	{
		LockOnWidget->ShowLockOn();
	}

	// LockOn Marker Create
	LockOnMarker = CreateWidget<UV12LockOnMarker>(this, LockOnMarkerClass);
	if (LockOnMarker)
	{
		LockOnMarker->AddToViewport(40);
		LockOnMarker->SetMarkerVisible(true);
	}

	//RootPrimitive = Cast<UPrimitiveComponent>(LockedTarget->GetRootComponent());
}

void AV12_the_gamePlayerController::ConfirmMissileFire()
{
	if (!bIsLockOnMode)
	{
		return;
	}

	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Locked Target"));
		return;
	}

	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn)
	{
		return;
	}

	AV12MissileItem* MissileItem =
		NewObject<AV12MissileItem>(this, PendingMissileItemClass);

	if (!MissileItem)
	{
		return;
	}

	MissileItem->SetTarget(LockedTarget);

	MissileItem->UseItem(OwnerPawn);

	CancelLockOn();

	if (InventoryComponent)
	{
		InventoryComponent->ConsumeCurrentItem();
	}
}

// ?�온 모드 ?�제
void AV12_the_gamePlayerController::CancelLockOn()
{
	if (!bIsLockOnMode)
	{
		return;
	}

	bIsLockOnMode = false;
	LockedTarget = nullptr;
	PendingMissileItemClass = nullptr;

	if (LockOnWidget)
	{
		LockOnWidget->HideLockOn();
		LockOnMarker->SetMarkerVisible(false);
	}

	LockOnMarker->RemoveFromParent();
	LockOnMarker = nullptr;

	UE_LOG(LogTemp, Log, TEXT("LockOn Canceled"));
}

void AV12_the_gamePlayerController::ChangeLockOnTarget()
{
	if (!bIsLockOnMode)
	{
		return;
	}

	CycleTarget();

	// ?�겟이 바뀌었?�면 UI 갱신
	if (LockOnWidget && LockedTarget)
	{
		LockOnWidget->ShowLockOn();
	}
}

#pragma endregion