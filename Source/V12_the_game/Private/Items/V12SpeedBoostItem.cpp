// Fill out your copyright notice in the Description page of Project Settings.
// V12SpeedBoostItem.cpp


#include "Items/V12SpeedBoostItem.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void AV12SpeedBoostItem::UseItem(AActor* TargetActor)
{
	Super::UseItem(TargetActor);

	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		if (UChaosWheeledVehicleMovementComponent* VehicleMovement = Pawn->FindComponentByClass<UChaosWheeledVehicleMovementComponent>())
		{
			// 차량 속도 증가 로직 구현
			UE_LOG(LogTemp, Warning, TEXT("Speed Boost!!"));
		}
	}
}
