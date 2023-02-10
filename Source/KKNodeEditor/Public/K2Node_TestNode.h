// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_TestNode.generated.h"

/**
 * 
 */
UCLASS()
class KKNODEEDITOR_API UK2Node_TestNode : public UK2Node
{
	GENERATED_BODY()

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	//~ Begin UEdGraphNode Interface.
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{
		return FText::FromString(TEXT("KKNode_TestNode"));
	}
	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinTypeChanged(UEdGraphPin* Pin) override;
	virtual FText GetTooltipText() const override
	{
		return FText::FromString(TEXT("This is a test node for KKNode"));
	}
	// 不知道干啥呀,调用非常频繁;
	virtual FText GetPinDisplayName(const UEdGraphPin* Pin) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface.
	virtual bool IsNodePure() const override { return true; }
	virtual bool NodeCausesStructuralBlueprintChange() const override { return true; }
	virtual void PostReconstructNode() override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual ERedirectType DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex) const override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override
	{
		return FText::FromString(TEXT("KKNode"));
	}
	//~ End UK2Node Interface.


	/*************************************** data *************************************/
	UPROPERTY()
	TArray<FString> PinNames;

	FName KK_GetFormatPinName()
	{
		return FName(TEXT("FormatStr"));
	}
	
	UEdGraphPin * FormatPin = nullptr;

	TArray<FString> KK_RegexFindValue(FString CheckString);
	bool KK_FindSameNamePin(FString InPinName);
	void KK_UpdatePinType();
};



