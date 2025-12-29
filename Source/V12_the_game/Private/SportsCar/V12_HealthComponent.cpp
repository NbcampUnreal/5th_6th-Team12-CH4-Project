#include "SportsCar/V12_HealthComponent.h"
#include "Net/UnrealNetwork.h"

UV12_HealthComponent::UV12_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UV12_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UV12_HealthComponent, CurrentHealth);
}


void UV12_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* OwnerActor = GetOwner();
	const ENetMode NetMode = GetNetMode();

	const TCHAR* NetModeStr =
		(NetMode == NM_DedicatedServer) ? TEXT("DedicatedServer") :
		(NetMode == NM_ListenServer) ? TEXT("ListenServer") :
		(NetMode == NM_Client) ? TEXT("Client") :
		TEXT("Standalone");

	UE_LOG(LogTemp, Warning,
		TEXT("[Health][BeginPlay] Owner=%s | NetMode=%s | HasAuthority=%d | CurrentHealth=%.1f"),
		*GetNameSafe(OwnerActor),
		NetModeStr,
		OwnerActor ? OwnerActor->HasAuthority() : 0,
		CurrentHealth
	);

	//체력 처리는 모두 서버에서 처리
	if (GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;

		UE_LOG(LogTemp, Warning,
			TEXT("[Health][Server Init] Owner=%s | Set Health = %.1f"),
			*GetNameSafe(OwnerActor),
			CurrentHealth
		);
	}
}

void UV12_HealthComponent::ApplyDamage(float Damage)
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Health][ApplyDamage] REJECTED (Client) Owner=%s"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	const float OldHealth = CurrentHealth;

	if (Damage <= 0.f) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Health][ApplyDamage] Damage <= 0"));
		return;
	}
	else if (CurrentHealth <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Health][ApplyDamage] CurrentHealth <= 0"));
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		//차량 파괴 로직
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[Health][Damage] Owner=%s | %.1f -> %.1f (Damage=%.1f)"),
		*GetNameSafe(GetOwner()),
		OldHealth,
		CurrentHealth,
		Damage
	);
}

void UV12_HealthComponent::ResetHealth()
{
	if (!GetOwner()->HasAuthority()) return;

	CurrentHealth = MaxHealth;
}

void UV12_HealthComponent::Heal(float Amount)
{
	if (!GetOwner()->HasAuthority()) return;

	if (Amount <= 0.f || CurrentHealth <= 0.f) return;

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
}

