#include "gamemode/V12_MainGameMode.h"
#include "Player/V12PlayerState.h"

void AV12_MainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	static int32 ColorIndex = 0;

	if (AV12PlayerState* PS = NewPlayer->GetPlayerState<AV12PlayerState>())
	{
		if (PresetColors.Num() > 0)
		{
			//BP에서 색 지정 후, 복제를 통해 자동 적용
			PS->VehicleColor = PresetColors[ColorIndex % PresetColors.Num()];
			ColorIndex++;
		}
	}
}
