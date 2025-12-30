// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/V12_tachoMeter.h"

void UV12_tachoMeter::UpdateRPM(float CurrentRPM)
{
	if (!IsValid(NeedleImage)) return;

	float ClampedRPM = FMath::Clamp(CurrentRPM, 0.f, 6000.f);
	float RPMRatio = ClampedRPM / 6000.f;
	float TargetAngle = FMath::Lerp(MinAngle, MaxAngle, RPMRatio);

	NeedleImage->SetRenderTransformAngle(TargetAngle);
}

void UV12_tachoMeter::UpdateSpeed(float currentSpeed)
{
	if (!IsValid(SpeedNeedleImage)) return;

	float FormattedSpeed = FMath::Abs(currentSpeed) * (0.036f);

	float ClampedSpeed = FMath::Clamp(FormattedSpeed, 0.f, 240.f);
	float SpeedRatio = ClampedSpeed / 240.f;
	float TargetAngle = FMath::Lerp(MinAngle, MaxAngle, SpeedRatio);

	SpeedNeedleImage->SetRenderTransformAngle(TargetAngle);
}

void UV12_tachoMeter::UpdateScore(int32 NewScore)
{
	if (!IsValid(NowScore)) return;
	NowScore->SetText(FText::AsNumber(NewScore));
}

void UV12_tachoMeter::UpdateCountdown(const FText& NewText)
{
	if (!IsValid(Countdown)) return;
	Countdown->SetText(NewText);
}
