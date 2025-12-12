// Fill out your copyright notice in the Description page of Project Settings.


#include "Swapper/MeshActorSwappingFunctions.h"

#include "Swapper/InstanceActorSwappingDataAsset.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UStaticMesh* UMeshActorSwappingFunctions::GetStaticMeshAsset(UInstancedStaticMeshComponent* Component)
{
    if (!Component)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::GetStaticMeshAsset >> Invalid ISMC")); 
        return nullptr;
    }
    return Component->GetStaticMesh();
}

UInstancedStaticMeshComponent* UMeshActorSwappingFunctions::FindISMCByPath(FName InstancingComponentPath)
{
    if (InstancingComponentPath == NAME_None)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::FindISMCByPath >> Invalid component path name."));
        return nullptr;
    }
    
    // Locate the original ISMC using the stored path name. 
    UInstancedStaticMeshComponent* ISMC = FindObject<UInstancedStaticMeshComponent>(
        nullptr, *InstancingComponentPath.ToString());

    if (!ISMC)
    {
        UE_LOG(SwapperComponent, Error, 
            TEXT("UMeshActorSwappingFunctions::FindISMCByPath >> Failed to find original ISMC at path: %s."), 
            *InstancingComponentPath.ToString());
    }
    return ISMC;
}

void UMeshActorSwappingFunctions::FinalizeSwapBack(
    UInstancedStaticMeshComponent* ISMC, 
    const FTransform& ChangedInstanceTransform, 
    int32 InstanceIndex, 
    UStaticMesh* FinalMeshAsset, 
    bool bDidChangeHappened)
{
    if (!ISMC || !FinalMeshAsset || InstanceIndex == INDEX_NONE)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::FinalizeSwapBack >> Invalid input (ISMC, Mesh, or Index). Aborting restore."));
        return;
    }

    // Set the new Static Mesh on the ISMC (This synchronizes the mesh type for all clients)
    ISMC->SetStaticMesh(FinalMeshAsset);

    // Add the instance back at the original index and new transform
    ISMC->AddInstance(ChangedInstanceTransform, true);

    UE_LOG(SwapperComponent, Log, 
        TEXT("UMeshActorSwappingFunctions::FinalizeSwapBack >> Instance restored to %s at index %d. State changed: %s. Final Mesh: %s"), 
        *ISMC->GetName(), 
        InstanceIndex, 
        (bDidChangeHappened ? TEXT("YES") : TEXT("NO")),
        *FinalMeshAsset->GetName());
}



bool UMeshActorSwappingFunctions::FindRuleForMeshAndIndex(UStaticMesh* Mesh, int32 InstanceIndex, 
    UInstanceActorSwappingDataAsset* SwapConfigDataAsset, FSwappingMeshActorPair& OutRulePair, 
    FName& OutTargetName, FName& OutContextName)
{
    if (!Mesh)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::FindRuleForMeshAndIndex >> Invalid Mesh"));
        return false;
    }
    if (!SwapConfigDataAsset)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::FindRuleForMeshAndIndex >> Cannot Get Valid DataAsset"));
        return false; 
    }
    
    for (const auto& TargetPair : SwapConfigDataAsset->SwappingInfoList)
    {
        const FName CurrentTargetName = TargetPair.Key;
        const FSwappingMeshActorInfo& MeshActorInfo = TargetPair.Value;
        
        for (const auto& ContextPair : MeshActorInfo.MeshActorMap) 
        {
            const FName CurrentContextName = ContextPair.Key;
            const FSwappingVariationMap& VariationMapWrapper = ContextPair.Value;
            const TMap<int32, FSwappingMeshActorPair>& VariationMap = VariationMapWrapper.VariationMap;
            
            for (const auto& VariationPair : VariationMap)
            {
                const int32 ConfigVariationIndex = VariationPair.Key; 
                const FSwappingMeshActorPair& RulePair = VariationPair.Value;
                
                if (RulePair.StaticMesh.IsValid() && RulePair.StaticMesh.Get() != Mesh)
                {
                    continue; 
                }
                if (ConfigVariationIndex != 0) 
                {
                    continue; 
                }
                
                OutRulePair = RulePair;
                OutTargetName = CurrentTargetName;
                OutContextName = CurrentContextName;

                UE_LOG(SwapperComponent, Log,
                    TEXT("UMeshActorSwappingFunctions::FindRuleForMeshAndIndex >> Match Found: Target=%s, Context=%s, ConfigIndex=%d (Instance Index was %d)"),
                    *CurrentTargetName.ToString(), *CurrentContextName.ToString(), ConfigVariationIndex, InstanceIndex);
                return true; 
            }
        }
    }
    
    UE_LOG(SwapperComponent, Error,
        TEXT("UMeshActorSwappingFunctions::FindRuleForMeshAndIndex >> No Match Found for Mesh (Instance Index %d)."),
        InstanceIndex);
    return false; 
}

bool UMeshActorSwappingFunctions::IsInstanceSwappable(
    UInstancedStaticMeshComponent* InstanceComponent, 
    int32 InstanceIndex,
    UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
    FName InteractionTag,
    FSwappingMeshActorPair& OutRulePair, 
    FName& OutTargetName, 
    FName& OutContextName)
{
    if (!InstanceComponent)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::IsInstanceSwappable >> Invalid InstanceComponent."));
        return false;
    }
    
    if (!InstanceComponent->ComponentTags.Contains(InteractionTag)) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::IsInstanceSwappable >> InstanceComponent %s missing InteractionTag: %s."),
            *InstanceComponent->GetName(), *InteractionTag.ToString());
        return false;
    }
    
    UStaticMesh* CurrentMesh = GetStaticMeshAsset(InstanceComponent);
    if (!CurrentMesh) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::IsInstanceSwappable >> Could not get Static Mesh for ISMC %s."),
            *InstanceComponent->GetName());
        return false;
    }
    
    if (!FindRuleForMeshAndIndex(
        CurrentMesh, 
        InstanceIndex, 
        SwapConfigDataAsset, 
        OutRulePair, 
        OutTargetName, 
        OutContextName))
    {
        //Log done in the function
        return false; 
    }
    
    if (!OutRulePair.ActorClassToSpawn)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::IsInstanceSwappable >> Rule found but ActorClassToSpawn is null for Mesh %s."),
            *CurrentMesh->GetName());
        return false;
    }
    
    UE_LOG(SwapperComponent, Log,
          TEXT("UMeshActorSwappingFunctions::IsInstanceSwappable >> %s is swappable (Rule: %s/%s)"),
          *CurrentMesh->GetName(), *OutTargetName.ToString(), *OutContextName.ToString());

    return true;
}

bool UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor(
    UInstancedStaticMeshComponent* InstanceComponent, 
    int32 InstanceIndex, 
    TSubclassOf<AActor> ActorClassToSpawn,
    AActor*& OutSpawnedActor,
    FName& OutOriginalMeshPath,
    FTransform& OutOriginalTransform,
    UStaticMesh*& OutOriginalStaticMesh)
{
    if (!InstanceComponent || InstanceIndex == INDEX_NONE || !ActorClassToSpawn)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor >> Invalid input. Aborting."));
        return false;
    }
    
    //Get Transform
    if (!InstanceComponent->GetInstanceTransform(InstanceIndex, OutOriginalTransform, true)) 
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor >> Failed to get transform for instance %d."),
            InstanceIndex);
        return false;
    }
    
    // Get the original mesh before removal!!!!! fuck
    OutOriginalStaticMesh = GetStaticMeshAsset(InstanceComponent);
    if (!OutOriginalStaticMesh)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor >> Failed to get Original Static Mesh. Aborting."));
        return false;
    }

    //Remove Instance
    if (!InstanceComponent->RemoveInstance(InstanceIndex))
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor >> Failed to remove instance %d."),
            InstanceIndex);
        return false;
    }

    //Spawn Actor Class 
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = InstanceComponent->GetOwner();
    AActor* NewActor = InstanceComponent->GetWorld()->SpawnActor<AActor>(ActorClassToSpawn, OutOriginalTransform, SpawnParams);

    if (!NewActor)
    {
        // Failed to spawn, reverse the removal
        InstanceComponent->AddInstance(OutOriginalTransform);
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor >> Failed to spawn actor. Instance restored."));
        return false;
    }

    //set output for the usage
    OutSpawnedActor = NewActor;
    OutOriginalMeshPath = FName(*InstanceComponent->GetPathName());
    
    UE_LOG(SwapperComponent, Log,
        TEXT("UMeshActorSwappingFunctions::ExecuteSwapInstanceToActor >> Instance %d removed. Actor %s spawned. Path: %s"), 
        InstanceIndex, *NewActor->GetName(), *OutOriginalMeshPath.ToString());

    return true;
}

bool UMeshActorSwappingFunctions::ExecuteSwapActorToInstance(
    FName InstancingComponentPath,
    int32 InstanceIndex,
    UStaticMesh* FinalMeshAsset,
    const FTransform& FinalTransform,
    bool bDidChangeHappened)
{
    // Locate the original ISMC
    UInstancedStaticMeshComponent* ISMC = FindISMCByPath(InstancingComponentPath);

    if (!ISMC)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::ExecuteSwapActorToInstance >> Failed to find ISMC via path. Aborting."));
        return false;
    }

    // Finalize the swap back
    FinalizeSwapBack(ISMC, FinalTransform, InstanceIndex, FinalMeshAsset, bDidChangeHappened);

    return true;
}