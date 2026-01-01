// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "V12GameInstance.generated.h"

class AV12_the_gamePlayerController;
class AV12PlayerState;

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API UV12GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    // �г��� �����?
    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerNickname;

    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetPlayerNickname(const FString& NewNickname)
    {
        PlayerNickname = NewNickname;
    }

    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetPlayerNickname() const { return PlayerNickname; }

    // �г����� ���õƴٰ� �����ϰ� �κ��?����
    UFUNCTION(BlueprintCallable)
    void JoinLobby();
	

#pragma region UI
public:
    UPROPERTY()
    int32 allPlayerCount = 0;

#pragma endregion
};
