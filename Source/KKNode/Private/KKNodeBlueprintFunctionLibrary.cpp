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

void UKKNodeBlueprintFunctionLibrary::KKNode_PrintInfo(FString Format,const TArray<FString> & Data)
{
	// FString str = FString::Join(Data,TEXT("->"));
	// UE_LOG(LogTemp,Warning,TEXT("format [%s] | content [%s]"),*Format,*str);
	FString tmp = Format;
	FRegexPattern Pattern(TEXT("\\{\\s*\\w+\\s*\\}"));
	FRegexMatcher Matcher(Pattern,Format);
	int i = 0 ;
	while (Matcher.FindNext())
	{
		if (Data.IsValidIndex(i))
		{
			tmp.ReplaceInline(*Matcher.GetCaptureGroup(0),*Data[i]);
		}
		i++;
	}
	UE_LOG(LogTemp,Warning,TEXT("%s"),*tmp);
}

