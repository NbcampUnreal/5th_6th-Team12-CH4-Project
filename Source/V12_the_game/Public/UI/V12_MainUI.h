#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h" 
#include "Components/MultiLineEditableTextBox.h"
#include "V12_MainUI.generated.h"

UCLASS()
class V12_THE_GAME_API UV12_MainUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ChatInputTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMultiLineEditableTextBox> ChatDisplayTextBox;

	/// 최대 채팅 표시 갯수
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	int32 MaxChatLines = 10;

	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void AddChatMessage(const FText& NewMessage);
};