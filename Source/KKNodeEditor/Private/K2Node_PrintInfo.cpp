// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_PrintInfo.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeArray.h"
#include "KismetCompiler.h"
#include "KKNodeBlueprintFunctionLibrary.h"
#include "Internationalization/Regex.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UK2Node_PrintInfo::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	UE_LOG(LogTemp, Warning,
	       TEXT("/*************************************** AllocateDefaultPins *************************************/"));

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	InputStrPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FName(TEXT("str")));

	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// 防止刷新节点时,pin消失
	for (FString it : PinNames)
	{
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, *it);
	}
}

void UK2Node_PrintInfo::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);
	// Modify();
	UE_LOG(LogTemp, Warning,
	       TEXT(
		       "/*************************************** PinConnectionListChanged *************************************/"
	       ));
	UE_LOG(LogTemp, Warning, TEXT("Pin Name : %s"), *Pin->GetName());
	// FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	// FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	KK_ResetPinType(Pin);
}

void UK2Node_PrintInfo::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);
	UE_LOG(LogTemp, Warning,
	       TEXT("/*************************************** PinDefaultValueChanged *************************************/"
	       ));
	UE_LOG(LogTemp, Warning, TEXT("Pin Name : %s | new string : %s"), *Pin->GetName(), *Pin->GetDefaultAsString());

	if (Pin != InputStrPin || Pin->LinkedTo.Num() != 0)
	{
		return;
	}

	// 移除其他的Pin,准备重新创建
	for (auto it = Pins.CreateIterator(); it; ++it)
	{
		UEdGraphPin* CheckPin = *it;
		if (CheckPin->PinName.IsEqual(UEdGraphSchema_K2::PN_Execute) || CheckPin->PinName.IsEqual(
			UEdGraphSchema_K2::PN_Then))
		{
			continue;
		}
		if (InputStrPin && CheckPin == InputStrPin && CheckPin->Direction == EGPD_Input)
		{
			continue;
		}
		CheckPin->MarkPendingKill();
		it.RemoveCurrent();
	}

	// asf{a}asfafa{b}asfa
	// 重新创建Pin
	TArray<FString> find_pin_name;
	KK_RegexName(Pin->GetDefaultAsString(), FString(TEXT("\\{\\s*\\w+\\s*\\}")), find_pin_name);

	PinNames.Reset();
	for (FString it : find_pin_name)
	{
		it.ReplaceInline(TEXT("{"),TEXT(""));
		it.ReplaceInline(TEXT("}"),TEXT(""));
		it.RemoveSpacesInline();
		// 跳过同名
		if (PinNames.Find(it) == INDEX_NONE)
		{
			PinNames.Add(it);
			CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, *it);
		}
	}

	GetGraph()->NotifyGraphChanged();
}

void UK2Node_PrintInfo::PinTypeChanged(UEdGraphPin* Pin)
{
	Super::PinTypeChanged(Pin);
	UE_LOG(LogTemp, Warning,
	       TEXT("/*************************************** PinTypeChanged *************************************/"));
	UE_LOG(LogTemp, Warning, TEXT("Pin Name : %s"), *Pin->GetName());
	KK_ResetPinType(Pin);
}

void UK2Node_PrintInfo::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	UClass* print_info_class = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(print_info_class))
	{
		UBlueprintNodeSpawner* bp_node_spawner = UBlueprintNodeSpawner::Create(print_info_class);
		check(bp_node_spawner!=nullptr);
		ActionRegistrar.AddBlueprintAction(print_info_class, bp_node_spawner);
	}
}

void UK2Node_PrintInfo::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	UE_LOG(LogTemp, Warning,
	       TEXT("/*************************************** ExpandNode *************************************/"));

	UEdGraphPin* ExecPin = GetExecPin();
	UEdGraphPin* ThenPin = FindPinChecked(UEdGraphSchema_K2::PN_Then, EGPD_Output);

	if (!InputStrPin)
	{
		KK_GetInputStrPin();
	}
	// 执行函数,传递String
	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallFunction->SetFromFunction(
		UKKNodeBlueprintFunctionLibrary::StaticClass()->FindFunctionByName(
			GET_FUNCTION_NAME_CHECKED(UKKNodeBlueprintFunctionLibrary, KKNode_PrintInfo)));
	CallFunction->AllocateDefaultPins();

	if (ExecPin && ThenPin)
	{
		UEdGraphPin* DataPin = CallFunction->FindPinChecked(FName(TEXT("Data")), EGPD_Input);
		UEdGraphPin* FormatPin = CallFunction->FindPinChecked(FName(TEXT("Format")), EGPD_Input);

		
		// MakeArray
		UK2Node_MakeArray* MakeArrayNode = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(this, SourceGraph);
		MakeArrayNode->AllocateDefaultPins();

		UEdGraphPin* ArrayOutPin = MakeArrayNode->GetOutputPin();
		ArrayOutPin->MakeLinkTo(DataPin);
		MakeArrayNode->PinConnectionListChanged(ArrayOutPin);

		CompilerContext.MovePinLinksToIntermediate(*InputStrPin, *FormatPin);
		CompilerContext.MovePinLinksToIntermediate(*ExecPin, *(CallFunction->GetExecPin()));
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *(CallFunction->GetThenPin()));
		
		// how to make array
		for (int i = 0; i < PinNames.Num(); ++i)
		{
			UEdGraphPin* tmp_pin = FindPinChecked(PinNames[i], EGPD_Input);
			const FName tmp_pc_name = tmp_pin->PinType.PinCategory;
			// MakeArray拥有一个默认的Input,所以第一次需要跳过;
			if (i > 0)
			{
				MakeArrayNode->AddInputPin();
			}
			// 手动构建一个默认Pin的名称,必须使用其默认格式
			FString CreatePinName = FString::Printf(TEXT("[%d]"), i);
			// 获取到刚才添加的 Pin
			UEdGraphPin* ArrayInputPin = MakeArrayNode->FindPinChecked(CreatePinName);
			
			
			if (tmp_pc_name.IsEqual(UEdGraphSchema_K2::PC_String))
			{
				UE_LOG(LogTemp,Warning,TEXT("find pc string value"));
				CompilerContext.MovePinLinksToIntermediate(*tmp_pin, *ArrayInputPin);
			}
			else if (tmp_pc_name.IsEqual(UEdGraphSchema_K2::PC_Int))
			{
				UE_LOG(LogTemp,Warning,TEXT("find pc int value"));
				UK2Node_CallFunction * ConvertFunc = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this,SourceGraph);
				ConvertFunc->SetFromFunction(UKismetStringLibrary::StaticClass()->FindFunctionByName(
					GET_MEMBER_NAME_CHECKED(UKismetStringLibrary,Conv_IntToString)));
				ConvertFunc->AllocateDefaultPins();
				UEdGraphPin * func_input_pin = ConvertFunc->FindPinChecked(TEXT("InInt"),EGPD_Input);
				UEdGraphPin * func_ret_pin = ConvertFunc->GetReturnValuePin();

				/*************************************** 要注意MoveLink和MakeLink的区别 *************************************/
				CompilerContext.MovePinLinksToIntermediate(*tmp_pin,*func_input_pin);
				func_ret_pin->MakeLinkTo(ArrayInputPin);
			}
			else if (tmp_pc_name.IsEqual(UEdGraphSchema_K2::PC_Float))
			{
				UE_LOG(LogTemp,Warning,TEXT("find pc float value"));
				UK2Node_CallFunction * ConvertFunc = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this,SourceGraph);
				ConvertFunc->SetFromFunction(UKismetStringLibrary::StaticClass()->FindFunctionByName(
					GET_MEMBER_NAME_CHECKED(UKismetStringLibrary,Conv_FloatToString)));
				ConvertFunc->AllocateDefaultPins();
				
				UEdGraphPin * func_input_pin = ConvertFunc->FindPinChecked(TEXT("InFloat"),EGPD_Input);
				UEdGraphPin * func_ret_pin = ConvertFunc->GetReturnValuePin();
				
				CompilerContext.MovePinLinksToIntermediate(*tmp_pin,*func_input_pin);
				func_ret_pin->MakeLinkTo(ArrayInputPin);
			}
			
		}
		
		
	}
	
	BreakAllNodeLinks();
}

void UK2Node_PrintInfo::PostReconstructNode()
{
	Super::PostReconstructNode();
	UE_LOG(LogTemp, Warning,
	       TEXT("/*************************************** PostReconstructNode *************************************/"));
	// 遍历所有Pin,重新设置类型;
	for (UEdGraphPin* it : Pins)
	{
		KK_ResetPinType(it);
	}
}

UK2Node::ERedirectType UK2Node_PrintInfo::DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex,
                                                                       const UEdGraphPin* OldPin,
                                                                       int32 OldPinIndex) const
{
	UE_LOG(LogTemp, Warning,
	       TEXT(
		       "/*************************************** DoPinsMatchForReconstruction *************************************/"
	       ));
	// UE_LOG(LogTemp,Warning,TEXT("new pin name [%s] index [%d] | old pin name [%s] index [%d]"),
	// 	*NewPin->PinName.ToString(),NewPinIndex,*OldPin->PinName.ToString(),OldPinIndex);
	return Super::DoPinsMatchForReconstruction(NewPin, NewPinIndex, OldPin, OldPinIndex);
}

bool UK2Node_PrintInfo::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin,
                                               FString& OutReason) const
{
	// UE_LOG(LogTemp,Warning,TEXT("/*************************************** IsConnectionDisallowed *************************************/"));
	// UE_LOG(LogTemp,Warning,TEXT("MyPin name [%s] , OtherPin name [%s] , Reason [%s]"),
	// 	*MyPin->GetName(),*OtherPin->GetName(),*OutReason);
	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

/*************************************** custom helper *************************************/
UEdGraphPin* UK2Node_PrintInfo::KK_GetInputStrPin()
{
	if (!InputStrPin)
	{
		InputStrPin = FindPinChecked(FName(TEXT("str")), EGPD_Input);
	}
	return InputStrPin;
}

void UK2Node_PrintInfo::KK_RegexName(FString Str, FString Rule, TArray<FString>& Result)
{
	FRegexPattern Pattern(Rule);
	FRegexMatcher Matcher(Pattern, Str);
	while (Matcher.FindNext())
	{
		Result.Add(Matcher.GetCaptureGroup(0));
	}
}


bool UK2Node_PrintInfo::KK_CheckPinNameExist(FString PinName)
{
	for (UEdGraphPin* it : Pins)
	{
		if (it->GetName() == PinName)
		{
			return true;
		}
	}
	return false;
}

void UK2Node_PrintInfo::KK_ResetPinType(UEdGraphPin* Pin)
{
	if (Pin == InputStrPin || Pin->Direction != EGPD_Input || Pin->PinName.IsEqual(UEdGraphSchema_K2::PN_Execute))
	{
		return;
	}
	// 这个节点没有连接
	bool bPinTypeChanged = false;
	if (Pin->LinkedTo.Num() == 0)
	{
		static const FEdGraphPinType WildcardPinType = FEdGraphPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None,
		                                                               nullptr, EPinContainerType::None, false,
		                                                               FEdGraphTerminalType());

		// Ensure wildcard
		if (Pin->PinType != WildcardPinType)
		{
			Pin->PinType = WildcardPinType;
			bPinTypeChanged = true;
		}
	}
	else
	{
		UEdGraphPin* linkedPin = Pin->LinkedTo[0];
		if (Pin->PinType != linkedPin->PinType)
		{
			Pin->PinType = linkedPin->PinType;
			bPinTypeChanged = true;
		}
	}

	if (bPinTypeChanged)
	{
		GetGraph()->NotifyGraphChanged();

		// 
		UBlueprint* Blueprint = GetBlueprint();
		if (!Blueprint->bBeingCompiled)
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
			Blueprint->BroadcastChanged();
		}
	}
}


UEdGraphPin* UK2Node_PrintInfo::FindOutputStructPinChecked(UEdGraphNode* Node)
{
	check(NULL != Node);
	UEdGraphPin* OutputPin = NULL;
	for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = Node->Pins[PinIndex];
		if (Pin && (EGPD_Output == Pin->Direction))
		{
			OutputPin = Pin;
			break;
		}
	}
	check(NULL != OutputPin);
	return OutputPin;
}