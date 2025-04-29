// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_META_TYPE(Name, MetaKey) \
		FName FNMetaUtils::Name ## Key = FName(TEXT(#MetaKey));
#define N_IMPLEMENT_META_TYPE_HEADER(Name) \
	public: \
		static bool Has ## Name(UEdGraphNode* Node) { return HasKey(Node, Name ## Key); } \
		static FString Get ## Name(UEdGraphNode* Node) { return GetData(Node, Name ## Key); } \
		static FString Get ## Name##Unsafe(UEdGraphNode* Node) { return GetDataUnsafe(Node, Name ## Key); } \
		static bool TryGet## Name(UEdGraphNode* Node, FString& OutValue) { return TryGetData(Node, Name ## Key, OutValue);  }; \
\
		static bool Has ## Name(UScriptStruct* ScriptStruct) { return HasKey(ScriptStruct, Name ## Key); } \
		static FString Get ## Name(UScriptStruct* ScriptStruct) { return GetData(ScriptStruct, Name ## Key); } \
		static FString Get ## Name##Unsafe(UScriptStruct* ScriptStruct) { return GetDataUnsafe(ScriptStruct, Name ## Key); } \
		static bool TryGet## Name(UScriptStruct* ScriptStruct, FString& OutValue) { return TryGetData(ScriptStruct, Name ## Key, OutValue);  }; \
\
		static bool Has ## Name(UStruct* Struct) { return HasKey(Struct, Name ## Key); } \
		static FString Get ## Name(UStruct* Struct) { return GetData(Struct, Name ## Key); } \
		static FString Get ## Name##Unsafe(UStruct* Struct) { return GetDataUnsafe(Struct, Name ## Key); } \
		static bool TryGet## Name(UStruct* Struct, FString& OutValue) { return TryGetData(Struct, Name ## Key, OutValue);  }; \
\
		static bool Has ## Name(UClass* Class) { return HasKey(Class, Name ## Key); } \
		static FString Get ## Name(UClass* Class) { return GetData(Class, Name ## Key); } \
		static FString Get ## Name##Unsafe(UClass* Class) { return GetDataUnsafe(Class, Name ## Key); } \
		static bool TryGet## Name(UClass* Class, FString& OutValue) { return TryGetData(Class, Name ## Key, OutValue);  }; \
	private: \
		static FName Name ## Key;

/**
 * Helper utilities for accessing metadata on objects.
 */
class NEXUSCOREEDITOR_API FNMetaUtils
{
public:
	static FString EmptyString;
	
	static bool HasKey(UEdGraphNode* Node, const FName& Key);
	static bool HasKey(const UScriptStruct* ScriptStruct, const FName& Key);
	static bool HasKey(const UStruct* Struct, const FName& Key);
	static bool HasKey(const UClass* Class, const FName& Key);
	
	static FString GetData(UEdGraphNode* Node, const FName& Key);
	static FString GetData(const UScriptStruct* ScriptStruct, const FName& Key);
	static FString GetData(const UStruct* Struct, const FName& Key);
	static FString GetData(const UClass* Class, const FName& Key);

	static bool TryGetData(UEdGraphNode* Node, const FName& Key, FString& OutValue);
	static bool TryGetData(const UScriptStruct* ScriptStruct, const FName& Key, FString& OutValue);
	static bool TryGetData(const UStruct* Struct, const FName& Key, FString& OutValue);
	static bool TryGetData(const UClass* Class, const FName& Key, FString& OutValue);

	static FString GetDataUnsafe(UEdGraphNode* Node, const FName& Key);
	static FString GetDataUnsafe(const UScriptStruct* ScriptStruct, const FName& Key);
	static FString GetDataUnsafe(const UStruct* Struct, const FName& Key);
	static FString GetDataUnsafe(const UClass* Class, const FName& Key);

	N_IMPLEMENT_META_TYPE_HEADER(ExternalDocumentation);
};
