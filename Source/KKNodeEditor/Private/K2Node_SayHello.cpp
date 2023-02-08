// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_SayHello.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "KKNodeBlueprintFunctionLibrary.h"

void UK2Node_SayHello::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	// CreatePin 的 Category必须正确,Name可以自定义
	CreatePin(EEdGraphPinDirection::EGPD_Input,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Execute);
	
	// right
	// CreatePin(EEdGraphPinDirection::EGPD_Input,UEdGraphSchema_K2::PC_Exec,TEXT("abc"));
	
	CreatePin(EEdGraphPinDirection::EGPD_Output,TEXT("exec"),UEdGraphSchema_K2::PN_Then);

	// error
	// CreatePin(EEdGraphPinDirection::EGPD_Output,TEXT("custom"),UEdGraphSchema_K2::PN_Then);

	// right
	// CreatePin(EEdGraphPinDirection::EGPD_Input,UEdGraphSchema_K2::PC_Float,TEXT("Num"));
	// CreatePin(EEdGraphPinDirection::EGPD_Output,UEdGraphSchema_K2::PC_String,TEXT("Str"));
	
}

void UK2Node_SayHello::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	UClass * say_hello_class = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(say_hello_class))
	{
		UBlueprintNodeSpawner * bp_node_spawner = UBlueprintNodeSpawner::Create(say_hello_class);
		check(bp_node_spawner!=nullptr);
		ActionRegistrar.AddBlueprintAction(say_hello_class,bp_node_spawner);
	}
}

void UK2Node_SayHello::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// 获取自身Node的Pin
	UEdGraphPin * ExecPin = GetExecPin();
	UEdGraphPin * ThenPin = FindPinChecked(UEdGraphSchema_K2::PN_Then);

	if (ExecPin && ThenPin)
	{
		// 获取函数名称
		FName CalledFuncName = GET_FUNCTION_NAME_CHECKED(UKKNodeBlueprintFunctionLibrary,KKNode_SayHello);

		// 创建 K2Node_CallFunction节点
		UK2Node_CallFunction * CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this,SourceGraph);
		// 设置执行函数
		CallFunction->FunctionReference.SetExternalMember(CalledFuncName,UKKNodeBlueprintFunctionLibrary::StaticClass());
		// K2Node_分配Pin
		CallFunction->AllocateDefaultPins();

		// 链接自身Pin和K2Node_CallFunction的Pin
		CompilerContext.MovePinLinksToIntermediate(*ExecPin,*CallFunction->GetExecPin());
		CompilerContext.MovePinLinksToIntermediate(*ThenPin,*CallFunction->GetThenPin());
	}
	BreakAllNodeLinks();
}
