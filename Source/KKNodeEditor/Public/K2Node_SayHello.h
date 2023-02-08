// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_SayHello.generated.h"

/**
 * 
 */
UCLASS()
class KKNODEEDITOR_API UK2Node_SayHello : public UK2Node
{
	GENERATED_BODY()

	/*************************************** UEdGraphNode interface *************************************/
	virtual FText GetTooltipText() const override
	{
		return FText::FromString(TEXT("Say Hello test function's tooltip"));
	};
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{
		return FText::FromString(TEXT("KKNode_SayHello")); 
	};

	// 添加执行Pin
	virtual void AllocateDefaultPins() override;


	
	/*************************************** UK2Node interface *************************************/
	virtual FText GetMenuCategory() const override
	{
		return FText::FromString(TEXT("KKNode"));
	};
	// 重载此函数,不然无法在右键菜单找到此节点
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	// 展开节点,真正的执行处理
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
