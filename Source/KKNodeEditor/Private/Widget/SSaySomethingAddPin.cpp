// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SSaySomethingAddPin.h"
#include "SlateOptMacros.h"
#include "Kismet2/BlueprintEditorUtils.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSaySomethingAddPin::Construct(const FArguments& InArgs)
{
	this->GraphNode = InArgs._SaySomethingNode;
	this->SetCursor(EMouseCursor::CardinalCross);
	this->UpdateGraphNode();
}

void SSaySomethingAddPin::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
	// 使用此函数创建Button估计会自动绑定下面的 OnAddPin() 函数
	TSharedRef<SWidget> add_widget = AddPinButtonContent(FText::FromString(TEXT("Add Line")),FText::FromString(TEXT("add one input pin")));
	
	InputBox->AddSlot()
	.AutoHeight()
	.VAlign(VAlign_Fill)
	.Padding(8.f,2.f,0,0)
	[
		add_widget
	];
}

FReply SSaySomethingAddPin::OnAddPin()
{
	UK2Node_SaySomething * node = Cast<UK2Node_SaySomething>(GraphNode);

	// 创建回滚操作,do/undo
	const FScopedTransaction Transaction(FText::FromString(TEXT("add line for say something node")));
	node->Modify();

	node->KK_AddPinNode();
	FBlueprintEditorUtils::MarkBlueprintAsModified(node->GetBlueprint());

	UpdateGraphNode();
	GraphNode->GetGraph()->NotifyGraphChanged();

	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
