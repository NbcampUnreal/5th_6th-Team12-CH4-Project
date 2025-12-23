// Fill out your copyright notice in the Description page of Project Settings.
// V12SpikeItem.cpp

#include "Items/V12SpikeItem.h"

void AV12SpikeItem::UseItem(AActor* TargetActor)
{
	UE_LOG(LogTemp, Error, TEXT("===== UseItem Called ====="));
	UE_LOG(LogTemp, Error, TEXT("This Actor Name : %s"), *GetName());
	UE_LOG(LogTemp, Error, TEXT("This Actor Class: %s"), *GetClass()->GetName());
	UE_LOG(LogTemp, Error, TEXT("HasAuthority   : %d"), HasAuthority());
	UE_LOG(LogTemp, Error, TEXT("SpikeTrapClass : %s"),
		SpikeTrapClass ? *SpikeTrapClass->GetName() : TEXT("NULL"));

	Super::UseItem(TargetActor);

	UE_LOG(LogTemp, Warning, TEXT("Spkike Trap!"));

	if (!TargetActor) return;

	if (!SpikeTrapClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnClass is NULL"));
		return;
	}

	FVector SpawnLocation = TargetActor->GetActorLocation();
	SpawnLocation -= TargetActor->GetActorForwardVector() * 400.f; // 뒤에 생성

	FRotator Rotation = TargetActor->GetActorRotation();

	GetWorld()->SpawnActor<AV12SpikeTrap>(SpikeTrapClass, SpawnLocation, Rotation);
}