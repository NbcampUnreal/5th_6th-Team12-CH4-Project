// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "V12LobbyWidget.generated.h"

class AV12PlayerState;

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API UV12LobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RebuildPlayerList();

    // BP에서 구현(디자인)할 이벤트들
    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void ClearPlayerRows();

    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void AddPlayerRow(AMyLobbyPlayerState* PlayerState);
	
};
