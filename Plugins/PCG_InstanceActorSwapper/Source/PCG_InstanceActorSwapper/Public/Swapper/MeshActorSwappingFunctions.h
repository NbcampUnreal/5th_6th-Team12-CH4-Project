#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Swapper/InstanceActorSwappingDataAsset.h" 
#include "MeshActorSwappingFunctions.generated.h"

// Forward Declares
class UInstancedStaticMeshComponent;
class AActor;
class UStaticMesh;


UCLASS()
class PCG_INSTANCEACTORSWAPPER_API UMeshActorSwappingFunctions : public UBlueprintFunctionLibrary
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
        FName& OutContextName
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
    
    UFUNCTION(BlueprintPure, Category = "Swapper|Logic")
    static bool FindRuleForMeshAndIndex(
        UStaticMesh* Mesh, 
        int32 InstanceIndex, 
        UInstanceActorSwappingDataAsset* SwapConfigDataAsset,
        FSwappingMeshActorPair& OutRulePair, 
        FName& OutTargetName, 
        FName& OutContextName
    );
    
    static UStaticMesh* GetStaticMeshAsset(UInstancedStaticMeshComponent* Component);

    static UInstancedStaticMeshComponent* FindISMCByPath(FName InstancingComponentPath);

    static void FinalizeSwapBack(
        UInstancedStaticMeshComponent* ISMC, 
        const FTransform& ChangedInstanceTransform, 
        int32 InstanceIndex, 
        UStaticMesh* FinalMeshAsset, 
        bool bDidChangeHappened
    );
};