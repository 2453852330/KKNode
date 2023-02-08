// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_SaySomething.generated.h"

/**
 * 
 */
UCLASS()
class KKNODEEDITOR_API UK2Node_SaySomething : public UK2Node
{
	GENERATED_BODY()

public:
	/*************************************** UEdGraphNode interface *************************************/
	virtual FText GetTooltipText() const override
	{
		return FText::FromString(TEXT("Say Something function's tooltip"));
	};
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{
		return FText::FromString(TEXT("KKNode_SaySomething")); 
	};

	// 添加执行Pin
	virtual void AllocateDefaultPins() override;

	// 使用自定义的 Widget -> AddPin
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;

	// 移除Pin时右键菜单:
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	void KK_RemovePin(UEdGraphPin * Pin);
	
	/*************************************** UK2Node interface *************************************/
	virtual FText GetMenuCategory() const override
	{
		return FText::FromString(TEXT("KKNode"));
	};
	// 重载此函数,不然无法在右键菜单找到此节点
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	// 展开节点,真正的执行处理
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// 重新创建Pin,不然重启编辑器,节点会失效;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	
	/*************************************** custom function *************************************/
	void KK_AddPinNode();

	// 注意: 此数据必须使用UPROPERTY()声明,不然会被GC导致链接错误
	UPROPERTY()
	TArray<FString> CachedPinNames;
	
};
