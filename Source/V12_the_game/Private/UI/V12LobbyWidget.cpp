#include "UI/V12LobbyWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Game/V12LobbyGameState.h"
#include "Player/V12PlayerState.h"

void UV12LobbyWidget::RebuildPlayerList()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AV12LobbyGameState* GS = World->GetGameState<AV12LobbyGameState>();
    if (!GS) return;

    ClearPlayerRows();

    for (APlayerState* PSBase : GS->PlayerArray)
    {
        if (AV12PlayerState* PS = Cast<AV12PlayerState>(PSBase))
        {
            AddPlayerRow(PS);
        }
    }
}