// Fill out your copyright notice in the Description page of Project Settings.
// V12DefenseItem.cpp


#include "Items/V12DefenseItem.h"
#include "V12_the_gamePawn.h"
#include "Items/V12HomingMissile.h"
#include "EngineUtils.h"
#include "TimerManager.h"


void AV12DefenseItem::UseItem(AActor* TargetActor)
{
	Super::UseItem(TargetActor);

	AV12_the_gamePawn* Pawn = Cast<AV12_the_gamePawn>(TargetActor);
	if (!Pawn)
	{
		return;
	}

	// Defense Enable
	Pawn->SetMissileDefense(true);

	// Eliminate missiles already in flight
	for (TActorIterator<AV12HomingMissile> It(GetWorld()); It; ++It)
	{
		if (It->GetHomingTarget() == Pawn)
		{
			It->Destroy();
		}
	}

	// Defense TimeOut
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &AV12DefenseItem::DisableDefense, Pawn);

	Pawn->GetWorldTimerManager().SetTimer(
		Pawn->MissileDefenseTimer,
		TimerDel,
		DefenseDuration,
		false
	);
}

void AV12DefenseItem::DisableDefense(AV12_the_gamePawn* Pawn)
{
	if (Pawn)
	{
		Pawn->SetMissileDefense(false);
	}
}
