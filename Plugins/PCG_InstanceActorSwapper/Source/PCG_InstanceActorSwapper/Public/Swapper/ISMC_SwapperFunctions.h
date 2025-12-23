#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Swapper/InstanceActorSwappingDataAsset.h" 
#include "ISMC_SwapperFunctions.generated.h"

// Forward Declares
class UInstancedStaticMeshComponent;
class AActor;
class UStaticMesh;


UCLASS()
class PCG_INSTANCEACTORSWAPPER_API UISMC_SwapperFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    
    UFUNCTION(BlueprintPure, Category = "Swapper|Logic|Validation")
    static bool IsInstanceSwappable(
        UInstancedStaticMeshComponent* InstanceComponent, 
        int32 InstanceIndex,
        UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
        FName InteractionTag,
        FSwappingMeshActorPair& OutRulePair, 
        FName& OutTargetName, 
        FName& OutContextName,
        int32& OutContextVariationIndex
    );
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Logic")
    static bool ExecuteSwapInstanceToActor(
        UInstancedStaticMeshComponent* InstanceComponent, 
        int32 InstanceIndex, 
        TSubclassOf<AActor> ActorClassToSpawn,
        AActor*& OutSpawnedActor,
        FName& OutOriginalMeshPath,
        FTransform& OutOriginalTransform,
        UStaticMesh*& OutOriginalStaticMesh 
    );
    
    UFUNCTION(BlueprintCallable, Category = "Swapper|Logic")
    static bool ExecuteSwapActorToInstance(
        FName InstancingComponentPath,
        int32 InstanceIndex,
        UStaticMesh* FinalMeshAsset,
        const FTransform& FinalTransform,
        bool bDidChangeHappened
    );

    UFUNCTION(BlueprintCallable, Category = "Swapper|Logic")
    static bool ExecuteSwapInstancedMesh_AtoB(
        UInstancedStaticMeshComponent* Original_ISMC,
        int32 InstanceIndex,
        UStaticMesh* TargetMeshAsset,
        const FTransform& TargetTransform);

    
    UFUNCTION(BlueprintPure, Category = "Swapper|Logic")
    static bool FindRuleForMeshAndIndex(
        UStaticMesh* Mesh,
        UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
        FSwappingMeshActorPair& OutRulePair,
        FName& OutTargetName,
        FName& OutContextName,
        int32& OutVariationIndex
    );

    UFUNCTION(BlueprintCallable, Category = "Swapper Functions")
    static bool FindRuleForContextAndTarget(
        UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
        FName TargetName,
        FName ContextName,
        FSwappingMeshActorPair& OutRulePair);
    
    static UStaticMesh* GetStaticMeshAsset(UInstancedStaticMeshComponent* Component);

    static UInstancedStaticMeshComponent* FindISMCByPath(FName InstancingComponentPath);


    static UInstancedStaticMeshComponent* FindOrCreateNewISMC(
        AActor* PCG_Volume,
        UStaticMesh* TargetMeshAsset);
    
    UFUNCTION(BlueprintCallable, Category = "Mesh Actor Swapping")
    static AActor* GetISMCOwnerActor(const FName& ComponentPathName);// in this pcg project, a pcg volume which have the ismc
};