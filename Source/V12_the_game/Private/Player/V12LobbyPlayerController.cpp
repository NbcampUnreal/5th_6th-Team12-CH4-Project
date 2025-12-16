#include "Player/V12LobbyPlayerController.h"
//#include "LobbyWidget.h"
#include "Game/V12GameInstance.h"
#include "Blueprint/UserWidget.h"
#include <Player/V12PlayerState.h>

void AV12LobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // �κ� UI ����
    //if (IsLocalController() && LobbyWidgetClass)
    //{
    //    LobbyWidgetInstance = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
    //    if (LobbyWidgetInstance)
    //    {
    //        LobbyWidgetInstance->AddToViewport();
    //    }
    //}

    // �г����� ������ ������
    if (IsLocalController())
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            if (UV12GameInstance* MyGI = Cast<UV12GameInstance>(GI))
            {
                if (!MyGI->PlayerNickname.IsEmpty())
                {
                    Server_SendNicknameToServer(MyGI->PlayerNickname);
                }
            }
        }
    }
}

void AV12LobbyPlayerController::Server_SendNicknameToServer_Implementation(const FString& InNickname)
{
    // �������� PlayerState�� �г��� ����
    if (AV12PlayerState* PS = GetPlayerState<AV12PlayerState>())
    {
        PS->SetPlayerNameOnServer(InNickname);
    }
}
