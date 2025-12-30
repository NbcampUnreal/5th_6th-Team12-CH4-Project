// Fill out your copyright notice in the Description page of Project Settings.


#include "gamemode/V12LobbyGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Player/V12PlayerState.h"

void AV12LobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer) return;

    if (AV12PlayerState* PS = NewPlayer->GetPlayerState<AV12PlayerState>())
    {
        if (PS->PlayerName.IsEmpty())
        {
            const int32 CurrentPlayerCount = GetNumPlayers();
            const FString TempName = FString::Printf(TEXT("Guest%d"), CurrentPlayerCount);
            PS->SetPlayerNameOnServer(TempName);
        }
    }
}
