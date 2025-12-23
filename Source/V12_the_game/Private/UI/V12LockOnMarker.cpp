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
	// 소용없는 함수들???
	//if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	//{
	//	CanvasSlot->SetPosition(ScreenPos);
	//}
	//SetPositionInViewport(ScreenPos, true);

	GEngine->AddOnScreenDebugMessage(
		-1,
		0.f,
		FColor::Green,
		FString::Printf(TEXT("ScreenPos: %s"), *ScreenPos.ToString()) // 타겟 위치 디버깅용
	);
}
