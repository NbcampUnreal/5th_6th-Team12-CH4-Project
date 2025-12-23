// Fill out your copyright notice in the Description page of Project Settings.
// V12ItemBase.cpp

#include "Items/V12ItemBase.h"


AV12ItemBase::AV12ItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AV12ItemBase::BeginPlay()
{
	Super::BeginPlay();
}

void AV12ItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AV12ItemBase::UseItem(class AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Item Used : %s"), *ItemName);
}