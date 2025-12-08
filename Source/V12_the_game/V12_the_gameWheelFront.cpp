// Copyright Epic Games, Inc. All Rights Reserved.

#include "V12_the_gameWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UV12_the_gameWheelFront::UV12_the_gameWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}