// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_SaySomething.h"
#include "KismetNodes/SGraphNodeK2Base.h"

// 用于添加Pin
class KKNODEEDITOR_API SSaySomethingAddPin : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS(SSaySomethingAddPin)
	{}
	SLATE_ARGUMENT(UK2Node_SaySomething*,SaySomethingNode)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;
	virtual FReply OnAddPin() override;
};
