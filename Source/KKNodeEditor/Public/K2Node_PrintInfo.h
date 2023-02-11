// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_PrintInfo.generated.h"

/**
 * 
 */
UCLASS()
class KKNODEEDITOR_API UK2Node_PrintInfo : public UK2Node
{
	GENERATED_BODY()
public:
	
	/*************************************** UEdGraphNode interface *************************************/
	virtual FText GetTooltipText() const override
	{
		return FText::FromString(TEXT("print info function's tooltip"));
	};
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{
		return FText::FromString(TEXT("KKNode_PrintInfo")); 
	};

	// 添加执行Pin : 节点创建时调用一次;
	// 右键刷新节点会调用
	virtual void AllocateDefaultPins() override;

	// 启用节点的细节属性面板;
	virtual bool ShouldShowNodeProperties() const override { return true; }

	// 当该节点的连线发生变化时调用;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	// 当Pin的值发生改变时调用;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	
	virtual void PinTypeChanged(UEdGraphPin* Pin) override;


	
	/*************************************** UK2Node interface *************************************/
	
	virtual FText GetMenuCategory() const override
	{
		return FText::FromString(TEXT("KKNode"));
	};
	virtual FLinearColor GetNodeTitleColor() const override
	{
		return FLinearColor(1.f,0,0);
	}
	// 重载此函数,不然无法在右键菜单找到此节点
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	
	// 展开节点,真正的执行处理:每次编译蓝图都会调用;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	virtual bool NodeCausesStructuralBlueprintChange() const override { return true; }
	
	// 右键刷新节点会调用
	virtual void PostReconstructNode() override;

	virtual ERedirectType DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex) const override;

	// 检测两个节点之间能否连接;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	/*************************************** save data *************************************/

	
	UEdGraphPin * InputStrPin = nullptr;

	UPROPERTY()
	TArray<FString> PinNames;

	/*************************************** custom helper *************************************/
	void KK_GetInputStrPin() const;

	// 此函数取消使用
	void KK_RegexName(FString Str,FString Rule,TArray<FString> & Result);

	bool KK_CheckPinNameExist(FString PinName);

	void KK_ResetPinType(UEdGraphPin * Pin);

	UEdGraphPin* FindOutputStructPinChecked(UEdGraphNode* Node);
	
	// 此数据可以在bp细节面板查看修改;
	UPROPERTY(EditAnywhere)
	int32 ShowLevel = 1;

	// 使用此函数匹配
	TArray<FString> KK_RegexFindValue(FString CheckString);
	bool KK_FindSameNamePin(FString InPinName);
};


