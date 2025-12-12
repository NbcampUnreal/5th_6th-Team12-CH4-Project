// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/V12_MainUI.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Containers/StringConv.h"

void UV12_MainUI::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(ChatInputTextBox))
    {
        ChatInputTextBox->OnTextCommitted.AddDynamic(this, &UV12_MainUI::OnChatTextCommitted);
    }

    // 기본값은 채팅창 안 보이게
    SetVisibility(ESlateVisibility::Hidden);

}

void UV12_MainUI::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    // 엔터 누르면
    if (CommitMethod == ETextCommit::OnEnter)
    {
        if (!Text.IsEmpty())
        {
            /// 서버에 채팅을 송신하는 내용을 추가
        }

        // 입력창 비우고 채팅 입력창 안보이게
        ChatInputTextBox->SetText(FText::GetEmpty());
        SetVisibility(ESlateVisibility::Hidden);

        // 게임 모드로 전환하여 캐릭터 조작 복구
        //if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        //{
        //    PC->SetInputMode(FInputModeGameOnly());
        //    PC->bShowMouseCursor = false;
        //}
    }
}

void UV12_MainUI::AddChatMessage(const FText& NewMessage)
{
    if (!IsValid(ChatDisplayTextBox))
    {
        return;
    }

    FString CurrentText = ChatDisplayTextBox->GetText().ToString();

    // 1. 현재 텍스트를 줄 단위로 분리 (줄 바꿈 문자는 \n, \r\n, \r 모두 처리)
    TArray<FString> ChatLines;
    CurrentText.ParseIntoArrayLines(ChatLines, true);

    // 2. 새 메시지 추가
    // 새 메시지는 이미 NewMessage에 포함되어 있으므로, 
    // 기존 텍스트에 NewMessage를 추가하기 전에 먼저 줄 수를 체크합니다.

    // 주의: 기존 로직에서는 새로운 메시지를 추가하기 전에 줄 수를 세었습니다.
    // 여기서는 새 메시지가 "추가될" 줄 수를 미리 계산하여 처리합니다.

    // 새 메시지를 ChatLines에 추가
    ChatLines.Add(NewMessage.ToString());

    // 3. 줄 수 검사 및 삭제
    // ChatLines 배열의 크기가 MaxChatLines를 초과하는지 확인
    while (ChatLines.Num() > MaxChatLines)
    {
        // 가장 오래된 메시지 (가장 윗 줄)인 0번 인덱스 삭제
        ChatLines.RemoveAt(0);
    }

    // 4. 배열을 다시 하나의 FString으로 합치기
    // 줄 바꿈 문자('\n')를 사용하여 모든 줄을 다시 연결합니다.
    FString NewDisplayText = FString::Join(ChatLines, TEXT("\n"));

    // 5. ChatDisplayTextBox에 업데이트된 텍스트 설정
    ChatDisplayTextBox->SetText(FText::FromString(NewDisplayText));

    // *팁: 스크롤 최신화*
    // MultiLineEditableTextBox는 자동으로 스크롤이 최신 줄로 내려가지 않을 수 있습니다.
    // 만약 스크롤이 자동으로 내려가게 하려면, ChatDisplayTextBox 대신 UScrollBox를 사용하여
    // 스크롤 박스의 ScrollToEnd() 함수를 호출하는 것이 더 안정적입니다.
}
