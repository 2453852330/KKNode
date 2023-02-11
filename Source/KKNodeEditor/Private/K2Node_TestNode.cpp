// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_TestNode.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "Internationalization/Regex.h"


/*************************************** Begin UObject Interface *************************************/
void UK2Node_TestNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// UE_LOG(LogTemp,Warning,TEXT("PostEditChangeProperty"));
}

/*************************************** Begin UEdGraphNode Interface. *************************************/
void UK2Node_TestNode::AllocateDefaultPins()
{
	// UE_LOG(LogTemp,Warning,TEXT("AllocateDefaultPins"));
	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Then);
	FormatPin = CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_String,KK_GetFormatPinName());
	for (FString it : PinNames)
	{
		CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Wildcard,*it);
	}
}

void UK2Node_TestNode::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);
	// UE_LOG(LogTemp,Warning,TEXT("PinConnectionListChanged"));
	KK_UpdatePinType();
}

void UK2Node_TestNode::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	// UE_LOG(LogTemp,Warning,TEXT("PinDefaultValueChanged"));
	// 使用这个处理,完美解决Pin断连和顺序问题;
	if (FormatPin && Pin==FormatPin && Pin->Direction==EGPD_Input && Pin->LinkedTo.Num() == 0)
	{
		PinNames.Reset();
		
		TArray<FString> NewPinNames = KK_RegexFindValue(Pin->GetDefaultAsString());
		// UE_LOG(LogTemp,Warning,TEXT("find %d new pin names"),NewPinNames.Num());
		for (int32 i = 0 ; i < NewPinNames.Num() ; ++i)
		{
			// 检查是否存在同名Pin
			if (!KK_FindSameNamePin(NewPinNames[i]))
			{
				CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Wildcard,*NewPinNames[i]);
			}
			PinNames.AddUnique(NewPinNames[i]);
		}

		// 移除无效Pin
		for (auto it = Pins.CreateIterator(); it ; ++it)
		{
			// 已有节点中需要删除的节点
			UEdGraphPin * CheckPin = *it;
			if (!CheckPin->PinName.IsEqual(UEdGraphSchema_K2::PN_Execute) &&
			    CheckPin != FormatPin &&
				CheckPin->Direction == EGPD_Input &&
				NewPinNames.Find(CheckPin->GetName()) == INDEX_NONE)
			{
				CheckPin->MarkPendingKill();
				it.RemoveCurrent();
			}
		}
		GetGraph()->NotifyGraphChanged();
	}
}

void UK2Node_TestNode::PinTypeChanged(UEdGraphPin* Pin)
{
	// UE_LOG(LogTemp,Warning,TEXT("PinTypeChanged"));
}


FText UK2Node_TestNode::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	// UE_LOG(LogTemp,Warning,TEXT("GetPinDisplayName"));
	// UE_LOG(LogTemp,Warning,TEXT("Pin's name : %s"),*Pin->GetName());
	return Super::GetPinDisplayName(Pin);
}



/*************************************** Begin UK2Node Interface. *************************************/

void UK2Node_TestNode::PostReconstructNode()
{
	Super::PostReconstructNode();
	// UE_LOG(LogTemp,Warning,TEXT("PostReconstructNode"));
}

void UK2Node_TestNode::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	// UE_LOG(LogTemp,Warning,TEXT("ExpandNode"));
}

UK2Node::ERedirectType UK2Node_TestNode::DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex,
	const UEdGraphPin* OldPin, int32 OldPinIndex) const
{
	// UE_LOG(LogTemp,Warning,TEXT("DoPinsMatchForReconstruction"));
	// UE_LOG(LogTemp,Warning,TEXT("NewPin's name : %s , NewPin's index : %d | OldPin's name : %s , OldPin's index : %d "),*NewPin->GetName(),NewPinIndex,*OldPin->GetName(),OldPinIndex);
	return Super::DoPinsMatchForReconstruction(NewPin, NewPinIndex, OldPin, OldPinIndex);
}

bool UK2Node_TestNode::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin,
	FString& OutReason) const
{
	// UE_LOG(LogTemp,Warning,TEXT("IsConnectionDisallowed"));
	// UE_LOG(LogTemp,Warning,TEXT("MyPin's name : %s , OtherPin's name : %s , Reason : %s "),*MyPin->GetName(),*OtherPin->GetName(),*OutReason);
	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}


void UK2Node_TestNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	UClass * Class = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(Class))
	{
		UBlueprintNodeSpawner * BPNode = UBlueprintNodeSpawner::Create(Class);
		check(BPNode!=nullptr);
		ActionRegistrar.AddBlueprintAction(Class,BPNode);
	}
}

/*************************************** data and helper *************************************/
TArray<FString> UK2Node_TestNode::KK_RegexFindValue(FString CheckString)
{
	TArray<FString> Tmp;
	FRegexPattern Pattern(TEXT("\\{\\s*\\w+\\s*\\}"));
	FRegexMatcher Matcher(Pattern,CheckString);
	FString Init;
	while (Matcher.FindNext())
	{
		Init = Matcher.GetCaptureGroup(0);
		Init.ReplaceInline(TEXT("{"),TEXT(""));
		Init.ReplaceInline(TEXT("}"),TEXT(""));
		Init.RemoveSpacesInline();
		Tmp.Add(Init);
	}
	return Tmp;
}

bool UK2Node_TestNode::KK_FindSameNamePin(FString InPinName)
{
	for (UEdGraphPin * it : Pins)
	{
		if (it->GetName().Equals(InPinName))
		{
			return true;
		}
	}
	return false;
}

void UK2Node_TestNode::KK_UpdatePinType()
{
	bool bIsUpdate = false;
	for (FString it : PinNames)
	{
		UEdGraphPin * FindPin = FindPinChecked(it,EGPD_Input);
		// Pin 有连接
		if (FindPin->LinkedTo.Num())
		{
			if (FindPin->PinType != FindPin->LinkedTo[0]->PinType)
			{
				FindPin->PinType = FindPin->LinkedTo[0]->PinType;
				bIsUpdate = true;
			}
		}
		else
		{
			static const FEdGraphPinType WildcardPinType = FEdGraphPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType());
			if (FindPin->PinType != WildcardPinType )
			{
				FindPin->PinType = WildcardPinType;
				bIsUpdate = true;
			}
		}
	}
	if (bIsUpdate)
	{
		GetGraph()->NotifyGraphChanged();
	}
}
