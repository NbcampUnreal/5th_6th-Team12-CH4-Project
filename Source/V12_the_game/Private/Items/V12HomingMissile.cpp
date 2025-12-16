// Fill out your copyright notice in the Description page of Project Settings.
// V12HomingMissile.cpp

#include "Items/V12HomingMissile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "V12_the_gameSportsCar.h"



AV12HomingMissile::AV12HomingMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(80.f);
	Collision->SetNotifyRigidBodyCollision(true);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_GameTraceChannel1); // Missile Collision Channel
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	RootComponent = Collision;

	// ProjectileMovement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 4000.f;
	ProjectileMovement->MaxSpeed = 4000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 15000.f;

	// Hit Event
	Collision->OnComponentHit.AddDynamic(this, &AV12HomingMissile::OnMissileHit);
}

void AV12HomingMissile::BeginPlay()
{
	Super::BeginPlay();

	// OwnerActor Collision Ignore
	if (AActor* OwnerActor = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(OwnerActor, true);
	}
	
}

// 코드 재확인 필요
void AV12HomingMissile::SetHomingTarget(AActor* NewTarget)
{
	HomingTarget = NewTarget;

	if (!ProjectileMovement || !NewTarget)
	{
		return;
	}

	// 1순위: 충돌 있는 PrimitiveComponent 찾기
	TArray<UPrimitiveComponent*> Components;
	NewTarget->GetComponents<UPrimitiveComponent>(Components);

	for (UPrimitiveComponent* Comp : Components)
	{
		if (Comp->IsCollisionEnabled())
		{
			ProjectileMovement->HomingTargetComponent = Comp;
			return;
		}
	}

	ProjectileMovement->HomingTargetComponent = NewTarget->GetRootComponent();
}

void AV12HomingMissile::OnMissileHit(
	UPrimitiveComponent* HitComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	if (bExploded)
	{
		return;
	}
	
	// 자기 자신, 오너 무시
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Target Hit!"));

	Explode();
}

void AV12HomingMissile::Explode()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UE_LOG(LogTemp, Warning, TEXT("Explode called: %s"), *GetName());

	// 이동 정지
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	ProjectileMovement->SetUpdatedComponent(nullptr);
	SetActorEnableCollision(false);

	// 타겟 연결 완전 해제
	if (ProjectileMovement)
	{
		ProjectileMovement->HomingTargetComponent = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 폭발 이펙트
	if (ExplosionEffectClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(
			ExplosionEffectClass,
			GetActorLocation(),
			FRotator::ZeroRotator,
			Params
		);
	}

	// 넉백 처리(차량 폰 클래스에서 함수를 만들고 호출하는 것으로 변경 예정)
	if (HomingTarget)
	{
		APawn* Pawn = Cast<APawn>(HomingTarget);
		if (!Pawn) return;

		AV12_the_gameSportsCar* Vehicle =
			Cast<AV12_the_gameSportsCar>(HomingTarget);

		if (Vehicle)
		{
			Vehicle->LaunchAndSpin(GetActorLocation());
		}
	}

	SetLifeSpan(0.1f);
}

void AV12HomingMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// DebugSphere
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		50.f,
		12,
		FColor::Red,
		false,
		0.1f
	);

	if (bExploded) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, GroundTraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (World->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params))
	{
		float TargetZ = Hit.Location.Z + DesiredAltitude;
		FVector CurrentLocation = GetActorLocation();

		float NewZ = FMath::FInterpTo(
			CurrentLocation.Z,
			TargetZ,
			DeltaTime,
			AltitudeInterpSpeed
		);

		SetActorLocation(
			FVector(CurrentLocation.X, CurrentLocation.Y, NewZ),
			false
		);
	}

	CheckArrival();
}

// 목표물 도착 판정 함수
void AV12HomingMissile::CheckArrival()
{
	if (bExploded || !IsValid(HomingTarget)) return;

	float Distance =
		FVector::Dist(GetActorLocation(), HomingTarget->GetActorLocation());

	if (Distance <= ArrivalRadius)
	{
		UE_LOG(LogTemp, Warning, TEXT("Arrival Explode"));

		Explode();
	}
}