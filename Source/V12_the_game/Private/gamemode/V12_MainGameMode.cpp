#include "gamemode/V12_MainGameMode.h"
#include "Player/V12PlayerState.h"
#include "V12_the_gamePlayerController.h"

void AV12_MainGameMode::BeginPlay()
{
	Super::BeginPlay();

	/// 점수 올라가는지 확인만 하기 위한 기능
	GetWorldTimerManager().SetTimer(
		TestTimer,
		this,
		&AV12_MainGameMode::TestFunc,
		2.0f,
		true);


	/// 정확한 플레이어 갯수를 체크하고 시작하도록 변경
	StartCountdown();
}

void AV12_MainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!IsValid(NewPlayer)) return;
	
	static int32 ColorIndex = 0;

	A_PC.Add(Cast<AV12_the_gamePlayerController>(NewPlayer));

    if (AV12PlayerState* PS = NewPlayer->GetPlayerState<AV12PlayerState>())
    {
		FString tempName = PS->PlayerName;

		if (PresetColors.Num() > 0)
		{
			//BP에서 색 지정 후, 복제를 통해 자동 적용
			PS->VehicleColor = PresetColors[ColorIndex % PresetColors.Num()];
			ColorIndex++;
		}

		UE_LOG(LogTemp, Error, TEXT("Player Name is %s"), *tempName);
    }
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Player Name is ERROR"));
	}

}

void AV12_MainGameMode::StartCountdown()
{
	countdownCount = 5;

	GetWorldTimerManager().SetTimer(
		CountdownTimer,
		this,
		&AV12_MainGameMode::CountdownFunc,
		1.0f,
		true);
}

void AV12_MainGameMode::CountdownFunc()
{
	for(auto a : A_PC)
	{
		if (!IsValid(a)) continue;
		AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(a);
		if (!IsValid(PC)) continue;
		PC->CountdownCheck(FText::AsNumber(countdownCount));
	}
	/// 끝나면 제거
	countdownCount--;
	if (countdownCount < 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimer);

		for (auto a : A_PC)
		{
			if (!IsValid(a)) continue;
			AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(a);
			if (!IsValid(PC)) continue;
			FText zeroBack = FText::GetEmpty();
			PC->CountdownCheck(zeroBack);
			PC->BeginRace();
		}
	}
}

void AV12_MainGameMode::TestFunc()
{
	for(auto a : A_PC)
	{
		if (!IsValid(a)) continue;
		AV12_the_gamePlayerController* PC = Cast<AV12_the_gamePlayerController>(a);
		if (!IsValid(PC)) continue;
		AV12PlayerState* PS = PC->GetPlayerState<AV12PlayerState>();
		if (!IsValid(PS)) continue;
		PS->PlayerScore += 10;
		// UE_LOG(LogTemp, Warning, TEXT("Player %s Score: %d"), *PS->PlayerName, PS->PlayerScore);
	}
}
