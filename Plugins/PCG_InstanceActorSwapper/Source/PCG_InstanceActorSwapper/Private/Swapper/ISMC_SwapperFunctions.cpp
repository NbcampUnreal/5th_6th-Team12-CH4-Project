// Fill out your copyright notice in the Description page of Project Settings.


#include "Swapper/ISMC_SwapperFunctions.h"

#include "Swapper/InstanceActorSwappingDataAsset.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UStaticMesh* UISMC_SwapperFunctions::GetStaticMeshAsset(UInstancedStaticMeshComponent* Component)
{
    if (!Component)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UMeshActorSwappingFunctions::GetStaticMeshAsset >> Invalid ISMC")); 
        return nullptr;
    }
    return Component->GetStaticMesh();
}

UInstancedStaticMeshComponent* UISMC_SwapperFunctions::FindISMCByPath(FName InstancingComponentPath)
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


bool UISMC_SwapperFunctions::ExecuteSwapInstancedMesh_AtoB(UInstancedStaticMeshComponent* Original_ISMC, int32 InstanceIndex,
    UStaticMesh* TargetMeshAsset, const FTransform& TargetTransform)
{
    if (!Original_ISMC || !TargetMeshAsset || InstanceIndex == INDEX_NONE)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::SwapInstancedMesh_AtoB >> Invalid input component, mesh, or index."));
        return false;
    }

    AActor* OwnerActor = Original_ISMC->GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::SwapInstancedMesh_AtoB >> Original Component has no Owner Actor."));
        return false;
    }
    
    if (!Original_ISMC->RemoveInstance(InstanceIndex))
    {
        UE_LOG(SwapperComponent, Error, // Changed to Error, as it's critical
            TEXT("UISMC_SwapperFunctions::SwapInstancedMesh_AtoB >> FAILED TO REMOVE instance %d from %s. Aborting swap to prevent duplication."),
            InstanceIndex, *Original_ISMC->GetName());
        return false; // Exit the function here!
    }

    // Find or Create the Matching ISMC for the TargetMeshAsset
    UInstancedStaticMeshComponent* TargetISMC = FindOrCreateNewISMC(OwnerActor, TargetMeshAsset);

    if (!TargetISMC)// is not valid
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::SwapInstancedMesh_AtoB >> Could not find or create Target ISMC for mesh %s."),
            *TargetMeshAsset->GetName());
        
        // --- ERROR CLEANUP: Restore the original instance if the target ISMC creation failed.
        Original_ISMC->AddInstance(TargetTransform, true);
        return false;
    }

    // Add the new instance to the target ISMC.
    TargetISMC->AddInstance(TargetTransform, true);
    
    //Force a visual refresh for clients/render system
    TargetISMC->MarkRenderStateDirty();

    UE_LOG(SwapperComponent, Log,
        TEXT("UISMC_SwapperFunctions::SwapInstancedMesh_AtoB >> Swapped to new ISMC: %s."),
        *TargetISMC->GetName());
    return true;
}

UInstancedStaticMeshComponent* UISMC_SwapperFunctions::FindOrCreateNewISMC(AActor* PCG_Volume,
    UStaticMesh* TargetMeshAsset)
{
    if (!PCG_Volume || !TargetMeshAsset) return nullptr;

    // Search for an existing ISMC with the TargetMeshAsset
    TArray<UInstancedStaticMeshComponent*> ISMComponents;
    PCG_Volume->GetComponents<UInstancedStaticMeshComponent>(ISMComponents);

    for (UInstancedStaticMeshComponent* ISMC : ISMComponents)
    {
        if (ISMC && ISMC->GetStaticMesh() == TargetMeshAsset)
        {
            // Found existing ISMC for this mesh type
            return ISMC;
        }
    }

    //not found, dynamically create a new one
    FName NewCompName = MakeUniqueObjectName(PCG_Volume, UInstancedStaticMeshComponent::StaticClass(), 
        FName(*FString::Printf(TEXT("ISM_%s_Dynamic"), *TargetMeshAsset->GetName())));

    UInstancedStaticMeshComponent* NewISMC = NewObject<UInstancedStaticMeshComponent>(PCG_Volume, NewCompName);
    
    if (!NewISMC) return nullptr;

    // Configure and register
    NewISMC->SetStaticMesh(TargetMeshAsset); 
    
    // Copy collision settings from a common existing component for safety/consistency
    if (ISMComponents.Num() > 0)
    {
        NewISMC->SetCollisionProfileName(ISMComponents[0]->GetCollisionProfileName());
    }
    
    NewISMC->RegisterComponent();

    // Attach/add to the actor's component list (optional, but good practice)
    PCG_Volume->AddInstanceComponent(NewISMC); 

    return NewISMC;
}

AActor* UISMC_SwapperFunctions::GetISMCOwnerActor(const FName& ComponentPathName)
{
    if (ComponentPathName == NAME_None)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::GetISMCOwnerActor >> PathName is NAME_None."));
        return nullptr;
    }

    FString PathString = ComponentPathName.ToString();

    // Attempt to load the component object using its full path which points at ISMC object
    UInstancedStaticMeshComponent* LoadedComponent = Cast<UInstancedStaticMeshComponent>(
        StaticLoadObject(UInstancedStaticMeshComponent::StaticClass(), nullptr, *PathString)
    );

    if (!LoadedComponent) // wrong path or casting failed
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::GetISMCOwnerActor >> Could not load component at path: %s. PCG Volume not found."),
            *PathString);
        return nullptr;
    }
    
    // The component's owner is the AActor it is attached to (the PCG Volume).
    AActor* OwnerActor = LoadedComponent->GetOwner();

    if (!OwnerActor)
    {
        // Should not happen if the component is valid, but safe to check.
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::GetISMCOwnerActor >> Loaded component has no owner (detatched)."));
    }
    
    return OwnerActor;
}


bool UISMC_SwapperFunctions::FindRuleForMeshAndIndex(UStaticMesh* Mesh, UInstanceActorSwappingDataAsset* SwapConfigDataAsset, 
                                                     FSwappingMeshActorPair& OutRulePair, FName& OutTargetName, FName& OutContextName, int32& OutVariationIndex)
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
        
        // 2. Loop through Interaction Contexts
        for (const auto& ContextPair : MeshActorInfo.MeshActorMap) 
        {
            const FName CurrentContextName = ContextPair.Key;
            // Get the list of rules from the wrapper struct
            const TArray<FSwappingTransformationRule>& Rules = ContextPair.Value.RuleList;
            
            // 3. Loop through the actual Transformation Rules
            for (const FSwappingTransformationRule& Rule : Rules)
            {
                // Check 1: Input Mesh Key MUST match the mesh the player touched
                if (!Rule.InputMeshKey.IsValid() || Rule.InputMeshKey.Get() != Mesh) 
                {
                    continue; // Input mesh does not match, check next rule
                }
                
                // Check 2: Variation Index (if needed, currently simplified)
                // If you had a mechanism to determine the specific variation (e.g., from PCG data), you'd check it here:
                // if (Rule.VariationIndex != SwapperComponentVariationIndex) { continue; }

                // Match found!
                OutRulePair = Rule.OutputRule; // This is the output (ResultStaticMesh, ActorClass, Flags)
                OutTargetName = CurrentTargetName;
                OutContextName = CurrentContextName;
                OutVariationIndex = Rule.VariationIndex; // Returns the index found in the rule

                UE_LOG(SwapperComponent, Log,
                    TEXT("UMeshActorSwappingFunctions::FindRuleForMeshAndIndex >> Match Found: Target=%s, Context=%s, Input Mesh=%s. Output Mesh: %s"),
                    *CurrentTargetName.ToString(), 
                    *CurrentContextName.ToString(), 
                    *Mesh->GetName(),
                    *Rule.OutputRule.ResultStaticMesh.LoadSynchronous()->GetName());
                return true; 
            }
        }
    }
    
    UE_LOG(SwapperComponent, Warning,
        TEXT("UMeshActorSwappingFunctions::FindRuleForMeshAndIndex >> No Match Found for Mesh: %s."),
        *Mesh->GetName());
    return false;
}

// In Swapper/ISMC_SwapperFunctions.cpp

bool UISMC_SwapperFunctions::FindRuleForContextAndTarget(
    UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
    FName TargetName,
    FName ContextName,
    FSwappingMeshActorPair& OutRulePair)
{
    if (!SwapConfigDataAsset)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::FindRuleForContextAndTarget >> Invalid DataAsset."));
        return false;
    }
    
    // Find the Target Name (Outer Map)
    const FSwappingMeshActorInfo* MeshActorInfo = SwapConfigDataAsset->SwappingInfoList.Find(TargetName);
    if (!MeshActorInfo)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("UISMC_SwapperFunctions::FindRuleForContextAndTarget >> TargetName '%s' not found in SwappingInfoList."),
            *TargetName.ToString());
        return false;
    }
    
    // Find the Context Name (Inner Map)
    const FSwappingVariationRules* RuleListWrapper = MeshActorInfo->MeshActorMap.Find(ContextName);
    if (!RuleListWrapper)
    {
        UE_LOG(SwapperComponent, Warning,
            TEXT("UISMC_SwapperFunctions::FindRuleForContextAndTarget >> ContextName '%s' not found under Target '%s'."),
            *ContextName.ToString(), *TargetName.ToString());
        return false;
    }
    
    if (RuleListWrapper->RuleList.Num() == 0)
    {
        UE_LOG(SwapperComponent, Warning,
            TEXT("UISMC_SwapperFunctions::FindRuleForContextAndTarget >> Rule list is empty for Target %s, Context %s."),
            *TargetName.ToString(), *ContextName.ToString());
        return false;
    }

    // 3. Return the Output Rule of the first entry found
    OutRulePair = RuleListWrapper->RuleList[0].OutputRule;

    UE_LOG(SwapperComponent, Log,
        TEXT("UISMC_SwapperFunctions::FindRuleForContextAndTarget >> Match Found: Target=%s, Context=%s. Final Mesh: %s"),
        *TargetName.ToString(),
        *ContextName.ToString(),
        *OutRulePair.ResultStaticMesh.LoadSynchronous()->GetName());

    return true;
}

bool UISMC_SwapperFunctions::IsInstanceSwappable(
    UInstancedStaticMeshComponent* InstanceComponent, 
    int32 InstanceIndex,
    UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
    FName InteractionTag,
    FSwappingMeshActorPair& OutRulePair, 
    FName& OutTargetName, 
    FName& OutContextName,
    int32& OutContextVariationIndex)
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
    
    if (!FindRuleForMeshAndIndex
        (CurrentMesh, SwapConfigDataAsset, OutRulePair, OutTargetName, OutContextName,OutContextVariationIndex))
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

bool UISMC_SwapperFunctions::ExecuteSwapInstanceToActor(
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

bool UISMC_SwapperFunctions::ExecuteSwapActorToInstance(
    FName InstancingComponentPath,
    int32 InstanceIndex,
    UStaticMesh* FinalMeshAsset,
    const FTransform& FinalTransform,
    bool bDidChangeHappened)
{
    // Resolve the PCG Volume Actor using the original ISMC path.
    AActor* PCG_Volume = GetISMCOwnerActor(InstancingComponentPath);

    if (!PCG_Volume)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> Failed to find PCG Volume via path. Cannot complete swap."));
        return false;
    }

    // Find or Create the target ISMC component on the PCG Volume for the FinalMeshAsset.
    UInstancedStaticMeshComponent* TargetISMC = FindOrCreateNewISMC(PCG_Volume, FinalMeshAsset);

    if (!TargetISMC)
    {
        UE_LOG(SwapperComponent, Error,
            TEXT("ExecuteSwapActorToInstance >> Failed to find/create target ISMC for mesh: %s. Aborting."),
            *FinalMeshAsset->GetName());
        return false;
    }
    
    //  Finalize the swap by adding the new instance to the *TargetISMC*.
    TargetISMC->AddInstance(FinalTransform, true);
    
    //Mark dirty for render update
    TargetISMC->MarkRenderStateDirty();

    UE_LOG(SwapperComponent, Log,
        TEXT("ExecuteSwapActorToInstance >> Actor swapped back. Instance added to Target ISMC: %s. Final Mesh: %s"),
        *TargetISMC->GetName(),
        *FinalMeshAsset->GetName());

    return true;
}