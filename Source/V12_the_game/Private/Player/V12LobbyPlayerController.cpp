#include "Player/V12LobbyPlayerController.h"
#include "UI/V12LobbyWidget.h"
#include "Game/V12GameInstance.h"
#include "Blueprint/UserWidget.h"
#include <Player/V12PlayerState.h>

void AV12LobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 로비 UI 띄우기
    if (IsLocalController() && LobbyWidgetClass)
    {
        LobbyWidgetInstance = CreateWidget<UV12LobbyWidget>(this, LobbyWidgetClass);
        if (LobbyWidgetInstance)
        {
            LobbyWidgetInstance->AddToViewport();
        }
    }

    // 닉네임을 서버로 보내기
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
    // 서버에서 PlayerState에 닉네임 저장
    if (AV12PlayerState* PS = GetPlayerState<AV12PlayerState>())
    {
        PS->SetPlayerNameOnServer(InNickname);
    }
}
