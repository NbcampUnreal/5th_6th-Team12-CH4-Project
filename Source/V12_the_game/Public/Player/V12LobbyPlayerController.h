// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "V12LobbyPlayerController.generated.h"

class ULobbyWidget;
/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12LobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    virtual void BeginPlay() override;

protected:
    //UPROPERTY(EditAnywhere, Category = "UI")
    //TSubclassOf<ULobbyWidget> LobbyWidgetClass;

    //UPROPERTY()
    //ULobbyWidget* LobbyWidgetInstance;

    // 클라 → 서버로 닉네임 보내기
    UFUNCTION(Server, Reliable)
    void Server_SendNicknameToServer(const FString& InNickname);
    void Server_SendNicknameToServer_Implementation(const FString& InNickname);
	
};
