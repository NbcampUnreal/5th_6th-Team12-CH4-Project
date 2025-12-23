// Fill out your copyright notice in the Description page of Project Settings.
// V12SpeedBoostItem.cpp

#include "Items/V12SpeedBoostItem.h"
#include "V12_the_gameSportsCar.h"


void AV12SpeedBoostItem::UseItem(AActor* TargetActor)
{
	Super::UseItem(TargetActor);

	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		//if (UChaosWheeledVehicleMovementComponent* VehicleMovement = Pawn->FindComponentByClass<UChaosWheeledVehicleMovementComponent>())
		AV12_the_gameSportsCar* Car = Cast<AV12_the_gameSportsCar>(TargetActor);
		if (!Car) return;

			UE_LOG(LogTemp, Warning, TEXT("Speed Boost!!"));

			Car->ActivateBoost(BoostPower);

			Destroy();
	}
}