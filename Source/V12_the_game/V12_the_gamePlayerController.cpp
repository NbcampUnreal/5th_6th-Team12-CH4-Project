// Copyright Epic Games, Inc. All Rights Reserved.


#include "V12_the_gamePlayerController.h"
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
#include "UI/V12_tachoMeter.h"

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

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}

	/// RPM Update
	if (IsValid(VehiclePawn) && IsValid(SpeedUI) && IsValid(VehiclePawn))
	{
		float nowRPM = VehiclePawn->ChaosVehicleMovement->GetEngineRotationSpeed();
		SpeedUI->UpdateRPM(nowRPM);
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
