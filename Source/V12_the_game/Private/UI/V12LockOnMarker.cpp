// Fill out your copyright notice in the Description page of Project Settings.
// V12LockOnMarker.cpp

#include "UI/V12LockOnMarker.h"
#include "Components/CanvasPanelSlot.h"


void UV12LockOnMarker::SetMarkerVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UV12LockOnMarker::UpdateScreenPosition(const FVector2D& ScreenPos)
{
}

void UV12LockOnMarker::SetTargetedActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
}
