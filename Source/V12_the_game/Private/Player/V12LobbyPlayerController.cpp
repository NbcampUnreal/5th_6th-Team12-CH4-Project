#include "Player/V12LobbyPlayerController.h"
#include "UI/V12LobbyWidget.h"
#include "Game/V12GameInstance.h"
#include "Blueprint/UserWidget.h"
#include <Player/V12PlayerState.h>

void AV12LobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    //// �κ� UI ����
    //if (IsLocalController() && LobbyWidgetClass)
    //{
    //    LobbyWidgetInstance = CreateWidget<UV12LobbyWidget>(this, LobbyWidgetClass);
    //    if (LobbyWidgetInstance)
    //    {
    //        LobbyWidgetInstance->AddToViewport();
    //    }
    //}

    //if (IsLocalController())
    //{
    //    if (UGameInstance* GI = GetGameInstance())
    //    {
    //        if (UV12GameInstance* MyGI = Cast<UV12GameInstance>(GI))
    //        {
    //            if (!MyGI->PlayerNickname.IsEmpty())
    //            {
    //                Server_SendNicknameToServer(MyGI->PlayerNickname);
    //            }
    //        }
    //    }
    //}
}


