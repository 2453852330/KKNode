// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KKNodeBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class KKNODE_API UKKNodeBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,meta=(BlueprintInternalUseOnly = true))
	static void KKNode_SayHello();

	UFUNCTION(BlueprintCallable,meta=(BlueprintInternalUseOnly = true))
	static void KKNode_SaySomething(const TArray<FString> & Data);

	UFUNCTION(BlueprintCallable,meta=(BlueprintInternalUseOnly = true))
	static void KKNode_PrintInfo(FString Format,const TArray<FString> & Data);

	// this is K2Node_TestNode's Run Function
	UFUNCTION(BlueprintCallable,meta=(BlueprintInternalUseOnly = true))
	static void KKNode_TestNode(FString Data);
};
