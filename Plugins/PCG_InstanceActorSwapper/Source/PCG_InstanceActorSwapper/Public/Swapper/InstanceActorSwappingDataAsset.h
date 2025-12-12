// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InstanceActorSwappingDataAsset.generated.h"

//Log for Swapper and SwappingHelper
PCG_INSTANCEACTORSWAPPER_API DECLARE_LOG_CATEGORY_EXTERN(SwapperComponent, Log, All);


// Forward Declares
class UStaticMesh;
class AActor;


USTRUCT(BlueprintType)
struct FSwappingMeshActorPair
{
	GENERATED_BODY()
	
	//Mesh to be instanced
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TSoftObjectPtr<UStaticMesh> StaticMesh = nullptr; 
    
	//Actor Class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TSubclassOf<AActor> ActorClassToSpawn = nullptr;
};

USTRUCT(BlueprintType)
struct FSwappingVariationMap
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TMap<int32/*variation index*/, FSwappingMeshActorPair> VariationMap;
};

USTRUCT(BlueprintType)
struct FSwappingMeshActorInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TMap<FName/*Interaction Context*/, FSwappingVariationMap/*Static mesh, Actor class*/> MeshActorMap;
};


UCLASS(BlueprintType)
class PCG_INSTANCEACTORSWAPPER_API UInstanceActorSwappingDataAsset : public UDataAsset
{
	GENERATED_BODY()

public: 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SwappingInfo")
	TMap<FName/*Target Name*/, FSwappingMeshActorInfo/*Target's Swapping info*/> SwappingInfoList;
};
