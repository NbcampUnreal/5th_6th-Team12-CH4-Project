// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "V12LobbyGameMode.generated.h"

class AV12_the_gamePlayerController;
/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12LobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AV12LobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
