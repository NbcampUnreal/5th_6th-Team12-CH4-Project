// Copyright Epic Games, Inc. All Rights Reserved.

#include "V12_the_gameWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UV12_the_gameWheelRear::UV12_the_gameWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}