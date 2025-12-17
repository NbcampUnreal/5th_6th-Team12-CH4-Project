// Fill out your copyright notice in the Description page of Project Settings.
// V12HomingMissile.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "V12HomingMissile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class V12_THE_GAME_API AV12HomingMissile : public AActor
{
	GENERATED_BODY()
	
public:	
	AV12HomingMissile();

	void SetHomingTarget(AActor* NewTarget);

	virtual void Tick(float DeltaTime) override;

	void CheckArrival();


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnMissileHit
	(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	// Explode + NockBack
	void Explode();

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	bool bVelocityChange = true;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<AActor> ExplosionEffectClass;

	UPROPERTY()
	AActor* HomingTarget;

	// 목표물 도착 반경
	UPROPERTY(EditDefaultsOnly, Category = "Missile|Flight")
	float ArrivalRadius = 200.f;

	// 비행 고도
	UPROPERTY(EditDefaultsOnly, Category = "Missile|Flight")
	float DesiredAltitude = 300.f;

	// 고도 보정 속도
	UPROPERTY(EditDefaultsOnly, Category = "Missile|Flight")
	float AltitudeInterpSpeed = 5.f;

	// 지형 체크 거리
	UPROPERTY(EditDefaultsOnly, Category = "Missile|Flight")
	float GroundTraceDistance = 2000.f;

	bool bExploded = false;
};
