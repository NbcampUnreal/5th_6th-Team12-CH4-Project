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
}

void AV12PlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AV12PlayerState, PlayerName);
    DOREPLIFETIME(AV12PlayerState, PlayerScore);
}