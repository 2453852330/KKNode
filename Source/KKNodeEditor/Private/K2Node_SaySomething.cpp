// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_SaySomething.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeArray.h"
#include "KismetCompiler.h"
#include "KKNodeBlueprintFunctionLibrary.h"
#include "ToolMenu.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widget/SSaySomethingAddPin.h"

void UK2Node_SaySomething::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Then);
}

TSharedPtr<SGraphNode> UK2Node_SaySomething::CreateVisualWidget()
{
	// 创建自定义widget控件,并传递自身组为参数,从而绑定 OnAddPin 事件;
	return SNew(SSaySomethingAddPin).SaySomethingNode(this);

	// 也可以在模块启动时声明:
	// void FBehaviorTreeEditorModule::StartupModule()
	// {
	// 	GraphPanelNodeFactory_BehaviorTree = MakeShareable( new FGraphPanelNodeFactory_BehaviorTree() );
	// 	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_BehaviorTree);
	// }
}

// 移除Pin右键菜单
void UK2Node_SaySomething::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);
	if (Context->bIsDebugging)
	{
		return;
	}
	
	if (Context->Pin)
	{
		Menu->AddMenuEntry(FName(TEXT("Remove Line")),
			FToolMenuEntry::InitMenuEntry(
				FName(TEXT("remove pin name")), // Name没啥用
				FText::FromString(TEXT("Remove This Line")), // 显示名称
				FText::FromString(TEXT("remove pin tooltip")),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateUObject(const_cast<UK2Node_SaySomething*>(this), &UK2Node_SaySomething::KK_RemovePin, const_cast<UEdGraphPin*>(Context->Pin))))
				);
	}
}

void UK2Node_SaySomething::KK_RemovePin(UEdGraphPin * Pin) 
{
	FScopedTransaction Transaction(FText::FromString(TEXT("remove pin for say something node")));
	Modify();
	
	CachedPinNames.Remove(Pin->GetName());
	RemovePin(Pin);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}


void UK2Node_SaySomething::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	UClass * say_something_class = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(say_something_class))
	{
		UBlueprintNodeSpawner * bp_node_spawner = UBlueprintNodeSpawner::Create(say_something_class);
		check(bp_node_spawner!=nullptr);
		ActionRegistrar.AddBlueprintAction(say_something_class,bp_node_spawner);
	}
}

void UK2Node_SaySomething::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin * ExecPin = GetExecPin();
	UEdGraphPin * ThenPin = FindPinChecked(UEdGraphSchema_K2::PN_Then);

	if (ExecPin && ThenPin)
	{
		FName CalledFuncName = GET_FUNCTION_NAME_CHECKED(UKKNodeBlueprintFunctionLibrary,KKNode_SaySomething);
		// 创建K2Node_CallFunction节点
		UK2Node_CallFunction * CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this,SourceGraph);
		CallFunction->FunctionReference.SetExternalMember(CalledFuncName,UKKNodeBlueprintFunctionLibrary::StaticClass());
		CallFunction->AllocateDefaultPins();

		// link pin
		CompilerContext.MovePinLinksToIntermediate(*ExecPin,*(CallFunction->GetExecPin()));
		CompilerContext.MovePinLinksToIntermediate(*ThenPin,*(CallFunction->GetThenPin()));

		// create param pin
		// 创建
		UK2Node_MakeArray * MakeArray = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(this,SourceGraph);
		MakeArray->AllocateDefaultPins();

		// 获取Array节点的输出Pin
		UEdGraphPin * ArrayOut = MakeArray->GetOutputPin();
		// 获取CallFunction的参数Pin
		UEdGraphPin * FuncArgPin = CallFunction->FindPinChecked(TEXT("Data"));
		// 将MakeArray的输出值链接到CallFunction的参数输入
		ArrayOut->MakeLinkTo(FuncArgPin);

		// this will set the "Make Array" node's type, only works if one pin is connected.
		MakeArray->PinConnectionListChanged(ArrayOut);

		// 遍历缓存的 PinName;
		for (int32 i = 0; i < CachedPinNames.Num() ; ++i)
		{
			// MakeArray拥有一个默认的Input,所以第一次需要跳过;
			if (i > 0)
			{
				// 首先添加一个InputPin
				// 给MakeArray节点创建一个默认Pin,此节点的默认名称就是 [index]
				// [0] [1] [2] [3] ...
				MakeArray->AddInputPin();
			}
			// 手动构建一个默认Pin的名称,必须使用其默认格式
			FString CreatePinName = FString::Printf(TEXT("[%d]"),i);
			// 获取到刚才添加的 Pin
			UEdGraphPin * ArrayInputPin = MakeArray->FindPinChecked(CreatePinName);
			UE_LOG(LogTemp,Warning,TEXT("Find Array Pin [%d] : [%s]"),i,ArrayInputPin?TEXT("Success"):TEXT("Failed"));
			// 通过缓存的PinName获取到自身的 Pin
			UEdGraphPin * MyInputPin = FindPinChecked(CachedPinNames[i],EGPD_Input);
			UE_LOG(LogTemp,Warning,TEXT("Find Self Pin [%d] : [%s]"),i,MyInputPin?TEXT("Success"):TEXT("Failed"));

			// 链接我们的输入到Array的Pin
			FPinConnectionResponse Response = CompilerContext.MovePinLinksToIntermediate(*MyInputPin,*ArrayInputPin);
			UE_LOG(LogTemp,Warning,TEXT("Pin Link : %s"),*Response.Message.ToString());
		}
	}
	BreakAllNodeLinks();
}

void UK2Node_SaySomething::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

	// UE_LOG(LogTemp,Warning,TEXT("------------ReallocatePinsDuringReconstruction------------"));
	// UE_LOG(LogTemp,Warning,TEXT("ReallocatePins"));
	// for (UEdGraphPin * it : OldPins)
	// {
	// 	UE_LOG(LogTemp,Warning,TEXT("old pin names : %s"),*it->GetName());
	// }
	// TArray<UEdGraphPin*> MyPins = GetAllPins();
	// UE_LOG(LogTemp,Warning,TEXT("current pin num = [%d]"),MyPins.Num());
	// for (UEdGraphPin * it : MyPins)
	// {
	// 	UE_LOG(LogTemp,Warning,TEXT("my pin names : %s"),*it->GetName());
	// }
	// UE_LOG(LogTemp,Warning,TEXT("cache pin name num :[%d] "),CachedPinNames.Num());
	// for (FString it : CachedPinNames)
	// {
	// 	UE_LOG(LogTemp,Warning,TEXT("cache pin name : %s"),*it);
	// }
	// UE_LOG(LogTemp,Warning,TEXT("-------------------------------------------------"));
	
	// 经过测试发现:
	// Exec 和 Then 仍然存在,所以我们需要重新添加 其他Pin
	// 同时发现 CachePinNames 会自动序列化,所以数据也是可用的
	// 直接利用该数据创建即可;
	for (FString it : CachedPinNames)
	{
		CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_String,*it);
	}
	
}

/*************************************** custom function -> add pin  *************************************/
void UK2Node_SaySomething::KK_AddPinNode()
{
	int32 tmp = CachedPinNames.Num();
	int32 check_num = 1;
	FString pin_name = FString::Printf(TEXT("line %d"),tmp+check_num);
	while (CachedPinNames.Find(pin_name) != -1)
	{
		pin_name = FString::Printf(TEXT("line %d"),tmp+ (++check_num));
	}
	CachedPinNames.Add(pin_name);
	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_String,*pin_name);
}
