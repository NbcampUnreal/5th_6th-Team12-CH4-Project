// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/V12PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "V12_the_gamePawn.h"

AV12PlayerState::AV12PlayerState()
{
    bReplicates = true;
}


/// Seamless travel
void AV12PlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);

    AV12PlayerState* NewPS = Cast<AV12PlayerState>(PlayerState);
    if (NewPS)
    {
        NewPS->PlayerName = this->PlayerName;
        NewPS->PlayerScore = this->PlayerScore;
        NewPS->VehicleColor = this->VehicleColor;
    }
}


void AV12PlayerState::SetPlayerNameOnServer(const FString& NewName)
{
    PlayerName = NewName;
}

void AV12PlayerState::OnRep_VehicleColor()
{
    UE_LOG(LogTemp, Warning,
        TEXT("[OnRep_VehicleColor] Color=%s"),
        *VehicleColor.ToString()
    );
}

void AV12PlayerState::OnRep_LobbyPlayerName()
{
}

void AV12PlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AV12PlayerState, PlayerName);
    DOREPLIFETIME(AV12PlayerState, VehicleColor);
    DOREPLIFETIME(AV12PlayerState, PlayerScore);
    DOREPLIFETIME(AV12PlayerState, VehicleColor);
}