// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMetaUtils.h"
#include "K2Node_CallFunction.h"

FString FNMetaUtils::EmptyString = TEXT("");

N_IMPLEMENT_META_TYPE(ExternalDocumentation, DocsURL)

bool FNMetaUtils::HasKey(UEdGraphNode* Node, const FName& Key)
{
	if (Node == nullptr) return false;
	if (const UK2Node_CallFunction* FunctionCall = Cast<UK2Node_CallFunction>(Node))
	{
		const UClass* ParentClass  = FunctionCall->FunctionReference.GetMemberParentClass();

		if (ParentClass == nullptr)	
		{
			return false;
		}

		const FName MemberName = FunctionCall->FunctionReference.GetMemberName();
		if (MemberName == NAME_None)
		{
			return false;
		}

		if (const UFunction* Function = ParentClass->FindFunctionByName(MemberName);
			Function != nullptr)
		{
			return Function->HasMetaData(Key);
		}
	}
	return false;
}

bool FNMetaUtils::HasKey(const UScriptStruct* ScriptStruct, const FName& Key)
{
	if (ScriptStruct == nullptr) return false;
	return ScriptStruct->HasMetaData(Key);	
}

bool FNMetaUtils::HasKey(const UStruct* Struct, const FName& Key)
{
	if (Struct == nullptr) return false;
	return Struct->HasMetaData(Key);	
}
bool FNMetaUtils::HasKey(const UClass* Class, const FName& Key)
{
	if (Class == nullptr) return false;
	return Class->HasMetaData(Key);
}

FString FNMetaUtils::GetData(UEdGraphNode* Node, const FName& Key)
{
	
	if (Node == nullptr) return EmptyString;
	if (const UK2Node_CallFunction* FunctionCall = Cast<UK2Node_CallFunction>(Node))
	{

		const UClass* ParentClass  = FunctionCall->FunctionReference.GetMemberParentClass();
		if (ParentClass == nullptr)	
		{
			return EmptyString;
		}

		const FName MemberName = FunctionCall->FunctionReference.GetMemberName();
		if (MemberName == NAME_None)
		{
			return EmptyString;
		}

		if (const UFunction* Function = ParentClass->FindFunctionByName(MemberName);
			Function != nullptr)
		{
			if (Function->HasMetaData(Key))
			{
				return Function->GetMetaData(Key);
			}
		}
	}
	return EmptyString;
}

FString FNMetaUtils::GetData(const UScriptStruct* ScriptStruct, const FName& Key)
{
	if (ScriptStruct->HasMetaData(Key))
	{
		return ScriptStruct->GetMetaData(Key);
	}
	return EmptyString;
}

FString FNMetaUtils::GetData(const UStruct* Struct, const FName& Key)
{
	if (Struct->HasMetaData(Key))
	{
		return Struct->GetMetaData(Key);
	}
	return EmptyString;
}
FString FNMetaUtils::GetData(const UClass* Class, const FName& Key)
{
	if (Class->HasMetaData(Key))
	{
		return Class->GetMetaData(Key);
	}
	return EmptyString;
}

bool FNMetaUtils::TryGetData(UEdGraphNode* Node, const FName& Key, FString& OutValue)
{
	if (!HasKey(Node, Key))
	{
		return false;
	}
	OutValue = GetDataUnsafe(Node, Key);
	return true;
}

bool FNMetaUtils::TryGetData(const UScriptStruct* ScriptStruct, const FName& Key, FString& OutValue)
{
	if (!ScriptStruct->HasMetaData(Key))
	{
		return false;
	}
	OutValue = ScriptStruct->GetMetaData(Key);
	return true;
}

bool FNMetaUtils::TryGetData(const UStruct* Struct, const FName& Key, FString& OutValue)
{
	if (!Struct->HasMetaData(Key))
	{
		return false;
	}
	OutValue = Struct->GetMetaData(Key);
	return true;
}
bool FNMetaUtils::TryGetData(const UClass* Class, const FName& Key, FString& OutValue)
{
	if (!Class->HasMetaData(Key))
	{
		return false;
	}
	OutValue = Class->GetMetaData(Key);
	return true;
}

FString FNMetaUtils::GetDataUnsafe(UEdGraphNode* Node, const FName& Key)
{
	if (const UK2Node_CallFunction* FunctionCall = Cast<UK2Node_CallFunction>(Node))
	{
		const UClass* ParentClass  = FunctionCall->FunctionReference.GetMemberParentClass();
		const UFunction* Function = ParentClass->FindFunctionByName(FunctionCall->FunctionReference.GetMemberName());
		return Function->GetMetaData(Key);
	}
	return EmptyString;
}

FString FNMetaUtils::GetDataUnsafe(const UScriptStruct* ScriptStruct, const FName& Key)
{
	return ScriptStruct->GetMetaData(Key);
}

FString FNMetaUtils::GetDataUnsafe(const UStruct* Struct, const FName& Key)
{
	return Struct->GetMetaData(Key);
}
FString FNMetaUtils::GetDataUnsafe(const UClass* Class, const FName& Key)
{
	return Class->GetMetaData(Key);
}
