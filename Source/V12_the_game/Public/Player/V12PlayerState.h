// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "V12PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class V12_THE_GAME_API AV12PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    AV12PlayerState();

    UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayerName, BlueprintReadOnly, Category = "Lobby")
    FString PlayerName;

    void SetPlayerNameOnServer(const FString& NewName);

    // 게임 매니저사용
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameManager")
    int32 PlayerScore = 0;


    UPROPERTY(ReplicatedUsing = OnRep_VehicleColor)
    FLinearColor VehicleColor;

    UFUNCTION()
    void OnRep_VehicleColor();

protected:
    UFUNCTION()
    void OnRep_LobbyPlayerName();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
