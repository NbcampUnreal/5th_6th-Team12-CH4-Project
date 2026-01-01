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
    /// Seamless travel
    virtual void CopyProperties(APlayerState* PlayerState) override
    {
        Super::CopyProperties(PlayerState);

        AV12PlayerState* NewPS = Cast<AV12PlayerState>(PlayerState);
        if (NewPS)
        {
            NewPS->PlayerName = this->PlayerName;
            NewPS->PlayerScore = this->PlayerScore;
            NewPS->VehicleColor = this->VehicleColor;
            NewPS->SpawnSlotIndex = this->SpawnSlotIndex;
        }
    }

    UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayerName, BlueprintReadWrite, Category = "Lobby")
    FString PlayerName;

    void SetPlayerNameOnServer(const FString& NewName);

    // 게임 매니?�?�용
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameManager")
    int32 PlayerScore = 0;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "StartPoint")
    int32 SpawnSlotIndex = 0;


    UPROPERTY(ReplicatedUsing = OnRep_VehicleColor, BlueprintReadWrite, Category = "Vehicle")
    FLinearColor VehicleColor;

    UFUNCTION()
    void OnRep_VehicleColor();

protected:
    UFUNCTION()
    void OnRep_LobbyPlayerName();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
