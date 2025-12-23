// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/V12GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UV12GameInstance::JoinLobby()
{
	FName LobbyMapName = TEXT("TestLevel");
	UGameplayStatics::OpenLevel(this, LobbyMapName);
}
