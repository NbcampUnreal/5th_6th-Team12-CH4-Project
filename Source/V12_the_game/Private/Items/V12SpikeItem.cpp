// Fill out your copyright notice in the Description page of Project Settings.
// V12SpikeItem.cpp

#include "Items/V12SpikeItem.h"

void AV12SpikeItem::UseItem(AActor* TargetActor)
{
	Super::UseItem(TargetActor);

	UE_LOG(LogTemp, Warning, TEXT("Spkike Trap!"));

	if (!TargetActor) return;

	FVector SpawnLocation = TargetActor->GetActorLocation();
	SpawnLocation -= TargetActor->GetActorForwardVector() * 300.f; // 뒤에 생성

	FRotator Rotation = TargetActor->GetActorRotation();

	GetWorld()->SpawnActor<AV12SpikeTrap>(SpikeTrapClass, SpawnLocation, Rotation);

}