// Fill out your copyright notice in the Description page of Project Settings.


#include "KKNodeBlueprintFunctionLibrary.h"

void UKKNodeBlueprintFunctionLibrary::KKNode_SayHello()
{
	UE_LOG(LogTemp, Warning, TEXT("KKNode_SayHello Run"));
}

void UKKNodeBlueprintFunctionLibrary::KKNode_SaySomething(const TArray<FString>& Data)
{
	FString str = FString::Join(Data,TEXT("->"));
	UE_LOG(LogTemp, Warning, TEXT("KKNode_SaySomething : %s "), *str);
}
