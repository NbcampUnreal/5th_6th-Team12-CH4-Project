// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/V12PlayerState.h"
#include "Net/UnrealNetwork.h"

AV12PlayerState::AV12PlayerState()
{
    bReplicates = true;
}


void AV12PlayerState::SetPlayerNameOnServer(const FString& NewName)
{
    PlayerName = NewName;
}

void AV12PlayerState::OnRep_LobbyPlayerName()
{
    // UI 갱신 트리거가 필요하면 여기에서 BP 이벤트 호출도 가능
}

void AV12PlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AV12PlayerState, PlayerName);
}