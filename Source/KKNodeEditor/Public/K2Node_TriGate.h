// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_TriGate.generated.h"

/**
 * 
 */
UCLASS()
class KKNODEEDITOR_API UK2Node_TriGate : public UK2Node
{
	GENERATED_BODY()

public:
	
	/*************************************** UEdGraphNode interface *************************************/
	virtual FText GetTooltipText() const override
	{
		return FText::FromString(TEXT("Tri Gate function's tooltip"));
	};
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{
		return FText::FromString(TEXT("KKNode_TriGate")); 
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
	
	virtual FNodeHandlingFunctor* CreateNodeHandler(FKismetCompilerContext& CompilerContext) const override;
};
