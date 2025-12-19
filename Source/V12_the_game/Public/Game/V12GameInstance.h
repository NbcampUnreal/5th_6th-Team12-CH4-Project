// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "V12GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API UV12GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    // 닉네임 저장용
    UPROPERTY(BlueprintReadWrite, Category = "Player")
    FString PlayerNickname;

    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetPlayerNickname(const FString& NewNickname)
    {
        PlayerNickname = NewNickname;
    }

    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetPlayerNickname() const { return PlayerNickname; }

    // 닉네임이 세팅됐다고 가정하고 로비로 입장
    UFUNCTION(BlueprintCallable)
    void JoinLobby();
	
};
