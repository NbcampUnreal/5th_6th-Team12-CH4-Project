// Copyright Epic Games, Inc. All Rights Reserved.


#include "V12_the_gamePlayerController.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
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
	bReplicates = true;
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

		}
		else {

			UE_LOG(LogV12_the_game, Error, TEXT("Could not spawn vehicle UI widget."));

		}
	}

	// Inventory Widget Create
	if (IsLocalPlayerController())
	{
		if (ItemHUDWidgetClass)
		{
			if (APlayerController* LocalController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				ItemWindowWidget = CreateWidget<UUserWidget>(LocalController, ItemHUDWidgetClass);

				ItemWindowWidget->AddToViewport();
			}
		}
	}

	// LockOn Widget Create
	if (IsLocalPlayerController())
	{
		if (LockOnWidgetClass)
		{
			LockOnWidget = CreateWidget<UV12LockOnWidget>(this, LockOnWidgetClass);
			if (LockOnWidget)
			{
				LockOnWidget->AddToViewport(50); // HUD보다 위
				LockOnWidget->HideLockOn();
			}
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

	// LOCAL UI UPDATE ONLY
	if (IsLocalController())
	{
		// 25.12.23. mpyi
		// beginplay is late, on multiplay game.
		// if VehiclePawn is null, try to get it again.
		if (!IsValid(VehiclePawn))
		{
			VehiclePawn = Cast<AV12_the_gamePawn>(GetPawn());

			if (!IsValid(VehiclePawn))
			{
				return;
			}
		}

		/// RPM Update
		if (IsValid(VehiclePawn) && IsValid(SpeedUI))
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

		// 락온 중이 아니니 아무 것도 안한다
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
	if (!IsLocalController())
	{
		return;
	}

	Server_ScanTargets();
}

void AV12_the_gamePlayerController::Server_ScanTargets_Implementation()
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
	if (!IsLocalController())
	{
		return;
	}

	Server_CycleTarget();
}

void AV12_the_gamePlayerController::Server_CycleTarget_Implementation()
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
}

void AV12_the_gamePlayerController::EnterLockOnMode()
{
	if (!IsLocalController())
	{
		return;
	}

	Server_EnterLockOnMode();
}

void AV12_the_gamePlayerController::Server_EnterLockOnMode_Implementation()
{
	bIsLockOnMode = true;
}

void AV12_the_gamePlayerController::ConfirmMissileFire()
{
	if (!IsLocalController())
	{
		return;
	}

	Server_ConfirmMissileFire();
}

void AV12_the_gamePlayerController::Server_ConfirmMissileFire_Implementation()
{
	if (!bIsLockOnMode || !LockedTarget)
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

	Server_CancelLockOn();

	if (InventoryComponent)
	{
		InventoryComponent->ConsumeCurrentItem();
	}
}

void AV12_the_gamePlayerController::Server_CancelLockOn_Implementation()
{
	bIsLockOnMode = false;
	LockedTarget = nullptr;
	PendingMissileItemClass = nullptr;
}

// 락온 모드 해제
void AV12_the_gamePlayerController::CancelLockOn()
{
	if (!IsLocalController())
	{
		return;
	}

	Server_CancelLockOn();
}

void AV12_the_gamePlayerController::ChangeLockOnTarget()
{
	if (!bIsLockOnMode)
	{
		return;
	}

	CycleTarget();

	// ? 겟이 바뀌었? 면 UI 갱신
	if (LockOnWidget && LockedTarget)
	{
		LockOnWidget->ShowLockOn();
	}
}

void AV12_the_gamePlayerController::OnRep_LockedTarget()
{
	if (!IsLocalController())
	{
		return;
	}

	CachedLockOnTarget = LockedTarget;


	if (LockOnMarker)
	{
		LockOnMarker->SetTargetedActor(LockedTarget);
	}
}

void AV12_the_gamePlayerController::OnRep_LockOnMode()
{
	UE_LOG(LogTemp, Warning,
		TEXT("OnRep_LockOnMode | IsLocal=%d | bIsLockOnMode=%d"),
		IsLocalController(),
		bIsLockOnMode
	);

	if (!IsLocalController())
	{
		return;
	}

	// LockOn Widget
	if (LockOnWidget)
	{
		if (bIsLockOnMode)
		{
			LockOnWidget->ShowLockOn();
		}
		else
		{
			LockOnWidget->HideLockOn();
		}
	}

	// LockOn Marker
	if (bIsLockOnMode)
	{
		if (!LockOnMarker && LockOnMarkerClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Create LockOnmarker."));

			LockOnMarker = CreateWidget<UV12LockOnMarker>(this, LockOnMarkerClass);

			if (LockOnMarker)
			{
				LockOnMarker->AddToViewport(40);
				LockOnMarker->SetMarkerVisible(true);	

				if (CachedLockOnTarget)
				{
					LockOnMarker->SetTargetedActor(CachedLockOnTarget);
				}
			}
		}
	}
	else
	{
		if (LockOnMarker)
		{
			UE_LOG(LogTemp, Warning, TEXT("Remove LockOnmarker."));

			LockOnMarker->SetMarkerVisible(false);
			LockOnMarker->RemoveFromParent();
			LockOnMarker = nullptr;
		}
	}
}

bool AV12_the_gamePlayerController::IsLockOnMode() const
{
	return bIsLockOnMode;
}

void AV12_the_gamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AV12_the_gamePlayerController, LockedTarget);
	DOREPLIFETIME(AV12_the_gamePlayerController, bIsLockOnMode);
}

#pragma endregion