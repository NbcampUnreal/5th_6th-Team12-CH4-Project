// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/V12_tachoMeter.h"

void UV12_tachoMeter::UpdateRPM(float CurrentRPM)
{
	if (!IsValid(NeedleImage)) return;

	// 1. RPM 제한 (0 ~ 6000)
	// 실제 RPM이 9000이어도 6000으로 고정됩니다.
	float ClampedRPM = FMath::Clamp(CurrentRPM, 0.f, 6000.f);

	// 2. 0~6000 사이의 비율 계산 (Alpha 값)
	float RPMRatio = ClampedRPM / 6000.f;

	// 3. 비율을 바탕으로 실제 회전 각도 계산 (Lerp 사용)
	// 0일 때 MinAngle, 6000일 때 MaxAngle이 됩니다.
	float TargetAngle = FMath::Lerp(MinAngle, MaxAngle, RPMRatio);

	// 4. 화살표 이미지의 회전값 설정
	NeedleImage->SetRenderTransformAngle(TargetAngle);
}
