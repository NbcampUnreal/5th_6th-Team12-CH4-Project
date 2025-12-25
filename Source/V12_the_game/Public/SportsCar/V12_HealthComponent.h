#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "V12_HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class V12_THE_GAME_API UV12_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UV12_HealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	//UI 사용시 OnRep 함수 추가
	UPROPERTY(EditAnywhere, Replicated, Category = "Health")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;

public:	
	UFUNCTION(BlueprintCallable)
	void ApplyDamage(float Damage);

	void ResetHealth();

	void Heal(float Amount);
	
	//getter function
	FORCEINLINE float GetHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE bool IsDead() const { return CurrentHealth <= 0.f; }
};
