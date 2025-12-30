// Fill out your copyright notice in the Description page of Project Settings.
// V12HomingMissile.cpp

#include "Items/V12HomingMissile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "V12_the_gameSportsCar.h"
#include "SportsCar/V12_HealthComponent.h"


AV12HomingMissile::AV12HomingMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Multiplayer Replication
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);

	// Collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(80.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_GameTraceChannel1); // Missile Collision Channel
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);

	RootComponent = Collision;

	// ProjectileMovement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 4000.f;
	ProjectileMovement->MaxSpeed = 4000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 30000.f;
	ProjectileMovement->SetUpdatedComponent(Collision);

	ProjectileMovement->SetIsReplicated(true);

	// Overlap Event
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AV12HomingMissile::OnMissileOverlap);

	// Default Damage
	DamageAmount = 10.f;
}

void AV12HomingMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || bExploded)
	{
		return;
	}

	// 타겟 유효성 검사
	if (!IsValid(HomingTarget))
	{
		Destroy();
		return;
	}

	AV12_the_gamePawn* TargetPawn = Cast<AV12_the_gamePawn>(HomingTarget);
	if (TargetPawn && TargetPawn->bMissileDefenseActive)
	{
		Destroy();
		return;
	}

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

	UWorld* World = GetWorld();
	if (!World) return;

	float DistanceToTarget = FVector::Dist(GetActorLocation(), HomingTarget->GetActorForwardVector());

	if (DistanceToTarget > 1000.f)
	{
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, GroundTraceDistance);
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(
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
	}

	CheckArrival();
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
	if (!HasAuthority())
	{
		return;
	}

	HomingTarget = NewTarget;

	if (ProjectileMovement && NewTarget)
	{
		ProjectileMovement->HomingTargetComponent = NewTarget->GetRootComponent();
	}
}

void AV12HomingMissile::OnMissileOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || bExploded)
	{
		return;
	}

	// 자기 자신, 오너 무시
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}


	if (OtherActor == HomingTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missile Overlap Hit : %s"), *OtherActor->GetName());

		Explode();
	}
}

void AV12HomingMissile::Explode()
{
	if (!HasAuthority() || bExploded)
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

	// 데미지 처리
	if (HomingTarget)
	{
		AV12_the_gamePawn* HitPawn = Cast<AV12_the_gamePawn>(HomingTarget);

		// 차량 폰 클래스만 처리
		if (HitPawn)
		{
			// HelthComponent 찾기
			UV12_HealthComponent* HealthComp = HitPawn->FindComponentByClass<UV12_HealthComponent>();
			if (!HealthComp)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Missile] HealthComponent not found on %s"), *GetNameSafe(HitPawn));
				return;
			}

			// 데미지 Apply
			HealthComp->ApplyDamage(DamageAmount);

			UE_LOG(LogTemp, Warning, TEXT("Missile Hit : %s | Damage = %.1f"), *GetNameSafe(HitPawn), DamageAmount);
		}
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

	// 차량 폰 클래스에서 AddImpulse로 공중에 뜨면서 뒤집어짐.
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

// 목표물 도착 판정 함수
void AV12HomingMissile::CheckArrival()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bExploded || !IsValid(HomingTarget))
	{
		return;
	}

	const float Distance =
		FVector::Dist(GetActorLocation(), HomingTarget->GetActorLocation());

	if (Distance <= ArrivalRadius)
	{
		UE_LOG(LogTemp, Warning, TEXT("Arrival Explode"));

		Explode();
	}
}

void AV12HomingMissile::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AV12HomingMissile, HomingTarget);
}