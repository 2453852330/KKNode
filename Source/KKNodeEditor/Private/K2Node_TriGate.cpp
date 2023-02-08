// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_TriGate.h"

#include "BlueprintNodeSpawner.h"

#include "BlueprintActionDatabaseRegistrar.h"

#include "KismetCompiler.h"

/*************************************** custom  *************************************/
#include "EdGraphUtilities.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetCompiledFunctionContext.h"
#include "KismetCompilerMisc.h"

namespace TriGatePN
{
	FName Input = TEXT("Integer");
	FName Positive = TEXT("Positive");
	FName Zero = TEXT("Zero");
	FName Negative = TEXT("Negative");
}

class FNodeHandle_TriGate: public FNodeHandlingFunctor
{
protected:
	TMap<UEdGraphNode * ,FBPTerminal *> BoolTermMap;
	FBPTerminal * LiteralZeroTerminal = nullptr;
public:
	FNodeHandle_TriGate(FKismetCompilerContext & InCompilerContext):FNodeHandlingFunctor(InCompilerContext){}
	
	virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		FNodeHandlingFunctor::RegisterNets(Context, Node);
		// 存储比较结果的bool变量
		FBPTerminal * BoolTerminal = Context.CreateLocalTerminal();
		BoolTerminal->Type.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		BoolTerminal->Source = Node;
		BoolTerminal->Name = Context.NetNameMap->MakeValidName(Node)+TEXT("_CmpResult");
		BoolTermMap.Add(Node,BoolTerminal);
		// 字面量"0"
		LiteralZeroTerminal = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
		LiteralZeroTerminal->bIsLiteral = true;
		LiteralZeroTerminal->Type.PinCategory = UEdGraphSchema_K2::PC_Int;
		LiteralZeroTerminal->Name = TEXT("0");
	}

	virtual void Compile(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		// 转换到我们的K2Node
		UK2Node_TriGate* MyNode = CastChecked<UK2Node_TriGate>(Node);

        // 查找输入的那个整数的Pin对应的Terminal
        UEdGraphPin* InputPin = Context.FindRequiredPinByName(Node, TriGatePN::Input, EGPD_Input);

        UEdGraphPin* PinToTry = FEdGraphUtilities::GetNetFromPin(InputPin);
        FBPTerminal** pInputTerm = Context.NetMap.Find(PinToTry);
		
        if (pInputTerm == nullptr)
        {
            CompilerContext.MessageLog.Error(TEXT("FKCHandler_TriGate: Failed to resolve term passed into"), InputPin);
            return;
        }

        FBPTerminal* InputTerm = *pInputTerm;

        // 查找三个输出Pin
        UEdGraphPin* PositivePin = MyNode->FindPin(TriGatePN::Positive, EGPD_Output);
        UEdGraphPin* ZeroPin = MyNode->FindPin(TriGatePN::Zero, EGPD_Output);
        UEdGraphPin* NegativePin = MyNode->FindPin(TriGatePN::Negative, EGPD_Output);

        // 临时bool变量的Terminal
        FBPTerminal* BoolTerm = BoolTermMap.FindRef(MyNode);

		// 获取 UClass
        UClass* MathLibClass = UKismetMathLibrary::StaticClass();
		// 获取 UFunction
        UFunction* CreaterFuncPtr = FindField<UFunction>(MathLibClass, "Greater_IntInt");
        UFunction* EqualFuncPtr = FindField<UFunction>(MathLibClass, "EqualEqual_IntInt");

        // Statement 1: 计算表达式 BoolTerm = Interger > 0
		// 检测输入的值是否>0,并将结果保存到BoolTerm里面;
        FBlueprintCompiledStatement& CallCreaterZero = Context.AppendStatementForNode(MyNode);
        CallCreaterZero.Type = KCST_CallFunction;
        CallCreaterZero.FunctionToCall = CreaterFuncPtr;
        CallCreaterZero.LHS = BoolTerm;
        CallCreaterZero.RHS.Add(InputTerm);
        CallCreaterZero.RHS.Add(LiteralZeroTerminal);

        // Statement 2: if(BoolTerm)
		// 检测BoolTerm的值的Statement
		// 跳转到哪里 ?
		// CallEqualZero.bIsJumpTarget = true;和 IfPositive.TargetLabel = &CallEqualZero;
        FBlueprintCompiledStatement& IfPositive = Context.AppendStatementForNode(Node);
        IfPositive.Type = KCST_GotoIfNot;
        IfPositive.LHS = BoolTerm;

        // Statement 3: 执行 Positive Pin -> 正数
		// 使用Statement
        FBlueprintCompiledStatement& ExecPositive = Context.AppendStatementForNode(Node);
        ExecPositive.Type = KCST_UnconditionalGoto;
		// 无条件跳转到 >0
        Context.GotoFixupRequestMap.Add(&ExecPositive, PositivePin);

        // Statement 4: 计算表达式 BoolTerm = Interger == 0
		// 此处是跳转点
		// 检测==0的Statement
        FBlueprintCompiledStatement& CallEqualZero = Context.AppendStatementForNode(MyNode);
        CallEqualZero.Type = KCST_CallFunction;
        CallEqualZero.FunctionToCall = EqualFuncPtr;
        CallEqualZero.LHS = BoolTerm;
        CallEqualZero.bIsJumpTarget = true;
        CallEqualZero.RHS.Add(InputTerm);
        CallEqualZero.RHS.Add(LiteralZeroTerminal);

		// 设定跳转点;缺少此句,编译崩溃
        IfPositive.TargetLabel = &CallEqualZero;

        // Statement 5: GotoIfNot(BoolTerm) ->负数
		// 检测BoolTerm的Statment
        FBlueprintCompiledStatement& IfZero = Context.AppendStatementForNode(Node);
        IfZero.Type = KCST_GotoIfNot;
        IfZero.LHS = BoolTerm;
		// 如果不是0,就跳转到NegativePin
        Context.GotoFixupRequestMap.Add(&IfZero, NegativePin);

		
        // Statement 6: 执行 Zero Pin -> 0
        FBlueprintCompiledStatement& ExecZero = Context.AppendStatementForNode(Node);
        ExecZero.Type = KCST_UnconditionalGoto;
		Context.GotoFixupRequestMap.Add(&ExecZero, ZeroPin);
		
	}
};

/*************************************** Node *************************************/
void UK2Node_TriGate::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	CreatePin(EEdGraphPinDirection::EGPD_Input,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Execute);
	// CreatePin(EEdGraphPinDirection::EGPD_Output,UEdGraphSchema_K2::PC_Exec,UEdGraphSchema_K2::PN_Then);

	// 输入的float
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, TriGatePN::Input);
	// 输出执行
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TriGatePN::Positive);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TriGatePN::Zero);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TriGatePN::Negative);
}

void UK2Node_TriGate::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	UClass * tri_gate_class = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(tri_gate_class))
	{
		UBlueprintNodeSpawner * bp_node_spawner = UBlueprintNodeSpawner::Create(tri_gate_class);
		check(bp_node_spawner!=nullptr);
		ActionRegistrar.AddBlueprintAction(tri_gate_class,bp_node_spawner);
	}
}

FNodeHandlingFunctor* UK2Node_TriGate::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FNodeHandle_TriGate(CompilerContext);
}
