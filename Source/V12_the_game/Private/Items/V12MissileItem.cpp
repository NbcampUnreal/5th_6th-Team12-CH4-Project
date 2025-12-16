// Fill out your copyright notice in the Description page of Project Settings.
// V12MissileItem.cpp

#include "Items/V12MissileItem.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Items/V12HomingMissile.h"
#include "V12_the_gamePlayerController.h"


void AV12MissileItem::UseItem(AActor* TargetActor)
{
	Super::UseItem(TargetActor);

	if (!TargetActor) return;

	APawn* OwnerPawn = Cast<APawn>(TargetActor);
	if (!OwnerPawn) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;

	AV12_the_gamePlayerController* V12PC =
		Cast<AV12_the_gamePlayerController>(PC);

	if (!V12PC) return;

	// 서버에서만 스폰
	if (!OwnerPawn->HasAuthority()) return;

	FActorSpawnParameters Params;
	Params.Owner = OwnerPawn;
	Params.Instigator = OwnerPawn;

	// 미사일 스폰 위치
	AV12HomingMissile* Missile = GetWorld()->SpawnActor<AV12HomingMissile>(
		MissileClass,
		OwnerPawn->GetActorLocation() + OwnerPawn->GetActorForwardVector() * 300.f + FVector(0.f, 0.f, 200.f),
		OwnerPawn->GetActorRotation(),
		Params
	);

	if (Missile)
	{
		// 락온 타겟 전달
		Missile->SetHomingTarget(V12PC->LockedTarget);
	}
}

void AV12MissileItem::SetTarget(AActor* InTarget)
{
	MissileTargetActor = InTarget;
}

// 자동 선택 타겟
AActor* AV12MissileItem::SelectTarget(AActor* OwnerActor) 
{ 
	return FindBestTarget(OwnerActor); 
} 

// 자동으로 타겟 찾기
AActor* AV12MissileItem::FindBestTarget(AActor* OwnerActor) 
{
	if (!OwnerActor) return nullptr;

	UWorld* World = OwnerActor->GetWorld(); 
	
	if (!World) 
	{ 
		return nullptr; 
	} 
	
	const FVector OwnerLoc = OwnerActor->GetActorLocation();
	const FVector OwnerForward = OwnerActor->GetActorForwardVector();
	
	float BestDot = 0.7f; // 전방 기준 
	
	AActor* BestTarget = nullptr; 
	
	for (TActorIterator<APawn> It(World); It; ++It) 
	{ 
		APawn* Pawn = *It; 
		
		if (!Pawn || Pawn == OwnerActor) continue; 
		
		FVector ToTarget = (Pawn->GetActorLocation() - OwnerLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(OwnerForward, ToTarget);
		
		if (Dot > BestDot) 
		{ 
			BestDot = Dot; BestTarget = Pawn; 
		} 
	} 
	return BestTarget; 
}