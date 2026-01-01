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

void UV12_tachoMeter::UpdateRank(int32 NewRank)
{
	if (!IsValid(NowRank)) return;
	

	FString Suffix;
	if (NewRank % 100 >= 11 && NewRank % 100 <= 13)
	{
		Suffix = TEXT("th");
	}
	else
	{
		switch (NewRank % 10)
		{
		case 1:  Suffix = TEXT("st"); break;
		case 2:  Suffix = TEXT("nd"); break;
		case 3:  Suffix = TEXT("rd"); break;
		default: Suffix = TEXT("th"); break;
		}
	}
	FString CombinedString = FString::FromInt(NewRank) + Suffix;
	NowRank->SetText(FText::FromString(CombinedString));
}

void UV12_tachoMeter::UpdateLap(int32 NewLap)
{
	if (!IsValid(NowLap)) return;
	NowLap->SetText(FText::AsNumber(NewLap));
}

void UV12_tachoMeter::UpdateFullLap(int32 NewLap)
{
	if (!IsValid(FullLap)) return;
	FullLap->SetText(FText::AsNumber(NewLap));
}

void UV12_tachoMeter::UpdateGearMsg(int32 NewGear)
{
	if (!IsValid(NowGear)) return;
	NowGear->SetText(FText::AsNumber(NewGear));
}

void UV12_tachoMeter::UpdateSpeedMsg(float CurrentSpeed)
{
	if (!IsValid(NowSpeed)) return;
	int FormattedSpeed = FMath::Abs(CurrentSpeed) * (0.036f);
	NowSpeed->SetText(FText::AsNumber(FormattedSpeed));
}
