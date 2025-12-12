// Fill out your copyright notice in the Description page of Project Settings.
// V12MissileItem.cpp

#include "Items/V12MissileItem.h"

void AV12MissileItem::UseItem(AActor* TargetActor)
{
	Super::UseItem(TargetActor);

	UE_LOG(LogTemp, Warning, TEXT("Missile Fired!"));
}
