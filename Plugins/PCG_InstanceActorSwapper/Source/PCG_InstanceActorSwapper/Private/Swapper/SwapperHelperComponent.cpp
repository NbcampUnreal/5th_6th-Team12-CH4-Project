// Fill out your copyright notice in the Description page of Project Settings.

#include "Swapper/SwapperHelperComponent.h"

#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Swapper/InstanceActorSwappingDataAsset.h"


USwapperHelperComponent::USwapperHelperComponent()
{
    PrimaryComponentTick.bCanEverTick = false; 
}

void USwapperHelperComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USwapperHelperComponent::ActivateComponent()
{
    SetComponentTickEnabled(true); 
    UE_LOG(SwapperComponent, Log,
        TEXT("Helper component tick enabled on Actor: %s"),
        *GetOwner()->GetName());
}

void USwapperHelperComponent::DeactivateComponent()
{
    SetComponentTickEnabled(false); 
    UE_LOG(SwapperComponent, Log,
        TEXT("Helper component tick disabled on Actor: %s"),
        *GetOwner()->GetName());
}

void USwapperHelperComponent::InitializeSwapContext(FName InTargetName, FName InContextName,
    UInstancedStaticMeshComponent* InInstancingComponent)
{
    if (!GetOwner())
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("USwapperHelperComponent::InitializeSwapContext >> Owner Actor is null."));
        return;
    }
    
    TargetName = InTargetName;
    CurrentContextName = InContextName;
    InstancingComponent = InInstancingComponent;
    
    UE_LOG(SwapperComponent, Log, 
        TEXT("USwapperHelperComponent::InitializeSwapContext >> Initialized %s. Target: %s, Initial Context: %s"), 
        *GetOwner()->GetName(), 
        *TargetName.ToString(), 
        *CurrentContextName.ToString());
    
    // Activation setup is done
    ActivateComponent();
}

void USwapperHelperComponent::SignalSwapCompletion()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(SwapperComponent, Error, TEXT("SignalSwapCompletion failed: Owner Actor is null."));
        return;
    }

    UE_LOG(SwapperComponent, Log, 
        TEXT("SignalSwapCompletion called on %s. Final Context: %s. Firing OnReadyToSwapBack delegate."), 
        *OwnerActor->GetName(), 
        *CurrentContextName.ToString());

    // Fire the delegate, telling USwapperComponent to call ExecuteSwapActorToInstance
    OnReadyToSwapBack.Broadcast(OwnerActor);
    
    // Deactivate the helper.
    DeactivateComponent(); 
}