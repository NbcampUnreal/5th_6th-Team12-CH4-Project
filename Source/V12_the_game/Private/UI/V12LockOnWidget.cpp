// Fill out your copyright notice in the Description page of Project Settings.
// V12LockOnWidget.cpp


#include "UI/V12LockOnWidget.h"
#include "UI/V12LockOnMarker.h"
#include "Components/VerticalBox.h"

void UV12LockOnWidget::ShowLockOn()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UV12LockOnWidget::HideLockOn()
{
	SetVisibility(ESlateVisibility::Hidden);
}
