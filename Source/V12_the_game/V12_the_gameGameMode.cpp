// Copyright Epic Games, Inc. All Rights Reserved.

#include "V12_the_gameGameMode.h"
#include "Player/V12PlayerState.h"
#include "V12_the_gamePlayerController.h"

AV12_the_gameGameMode::AV12_the_gameGameMode()
{
	PlayerControllerClass = AV12_the_gamePlayerController::StaticClass();
}

