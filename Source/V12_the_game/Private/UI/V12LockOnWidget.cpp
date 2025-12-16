// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/V12LockOnWidget.h"
#include "Components/VerticalBox.h"

void UV12LockOnWidget::LockOnWidgetShow(bool bShow)
{
	if (!VerticalBox_LockOn)
	{
		return;
	}

	VerticalBox_LockOn->SetVisibility(
		bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden
	);
}

void UV12LockOnWidget::UpdateLockOnScreenPos(const FVector2D& ScreenPos)
{
	SetPositionInViewport(ScreenPos, true);
}
