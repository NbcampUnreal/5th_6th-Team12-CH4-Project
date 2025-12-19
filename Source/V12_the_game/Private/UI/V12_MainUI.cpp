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

    // �⺻���� ä��â �� ���̰�
    SetVisibility(ESlateVisibility::Hidden);

}

void UV12_MainUI::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    // ���� ������
    if (CommitMethod == ETextCommit::OnEnter)
    {
        if (!Text.IsEmpty())
        {
            /// ������ ä���� �۽��ϴ� ������ �߰�
        }

        // �Է�â ���� ä�� �Է�â �Ⱥ��̰�
        ChatInputTextBox->SetText(FText::GetEmpty());
        SetVisibility(ESlateVisibility::Hidden);

        // ���� ���� ��ȯ�Ͽ� ĳ���� ���� ����
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

    // 1. ���� �ؽ�Ʈ�� �� ������ �и� (�� �ٲ� ���ڴ� \n, \r\n, \r ��� ó��)
    TArray<FString> ChatLines;
    CurrentText.ParseIntoArrayLines(ChatLines, true);

    // 2. �� �޽��� �߰�
    // �� �޽����� �̹� NewMessage�� ���ԵǾ� �����Ƿ�, 
    // ���� �ؽ�Ʈ�� NewMessage�� �߰��ϱ� ���� ���� �� ���� üũ�մϴ�.

    // ����: ���� ���������� ���ο� �޽����� �߰��ϱ� ���� �� ���� �������ϴ�.
    // ���⼭�� �� �޽����� "�߰���" �� ���� �̸� ����Ͽ� ó���մϴ�.

    // �� �޽����� ChatLines�� �߰�
    ChatLines.Add(NewMessage.ToString());

    // 3. �� �� �˻� �� ����
    // ChatLines �迭�� ũ�Ⱑ MaxChatLines�� �ʰ��ϴ��� Ȯ��
    while (ChatLines.Num() > MaxChatLines)
    {
        // ���� ������ �޽��� (���� �� ��)�� 0�� �ε��� ����
        ChatLines.RemoveAt(0);
    }

    // 4. �迭�� �ٽ� �ϳ��� FString���� ��ġ��
    // �� �ٲ� ����('\n')�� ����Ͽ� ��� ���� �ٽ� �����մϴ�.
    FString NewDisplayText = FString::Join(ChatLines, TEXT("\n"));

    // 5. ChatDisplayTextBox�� ������Ʈ�� �ؽ�Ʈ ����
    ChatDisplayTextBox->SetText(FText::FromString(NewDisplayText));

    // *��: ��ũ�� �ֽ�ȭ*
    // MultiLineEditableTextBox�� �ڵ����� ��ũ���� �ֽ� �ٷ� �������� ���� �� �ֽ��ϴ�.
    // ���� ��ũ���� �ڵ����� �������� �Ϸ���, ChatDisplayTextBox ��� UScrollBox�� ����Ͽ�
    // ��ũ�� �ڽ��� ScrollToEnd() �Լ��� ȣ���ϴ� ���� �� �������Դϴ�.
}
