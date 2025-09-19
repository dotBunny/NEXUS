// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_META_TYPE(Name, MetaKey) \
		FName FNMetaUtils::Name ## Key = FName(TEXT(#MetaKey));
#define N_IMPLEMENT_META_TYPE_HEADER(Name) \
	public: \
		static bool Has ## Name(UEdGraphNode* Node) { return FNMetaUtils::HasKey(Node, Name ## Key); } \
		static FString Get ## Name(UEdGraphNode* Node) { return FNMetaUtils::GetData(Node, Name ## Key); } \
		static FString Get ## Name##Unsafe(UEdGraphNode* Node) { return FNMetaUtils::GetDataUnsafe(Node, Name ## Key); } \
		static bool TryGet## Name(UEdGraphNode* Node, FString& OutValue) { return FNMetaUtils::TryGetData(Node, Name ## Key, OutValue);  }; \
\
		static bool Has ## Name(UScriptStruct* ScriptStruct) { return FNMetaUtils::HasKey(ScriptStruct, Name ## Key); } \
		static FString Get ## Name(UScriptStruct* ScriptStruct) { return FNMetaUtils::GetData(ScriptStruct, Name ## Key); } \
		static FString Get ## Name##Unsafe(UScriptStruct* ScriptStruct) { return FNMetaUtils::GetDataUnsafe(ScriptStruct, Name ## Key); } \
		static bool TryGet## Name(UScriptStruct* ScriptStruct, FString& OutValue) { return FNMetaUtils::TryGetData(ScriptStruct, Name ## Key, OutValue);  }; \
\
		static bool Has ## Name(UStruct* Struct) { return FNMetaUtils::HasKey(Struct, Name ## Key); } \
		static FString Get ## Name(UStruct* Struct) { return FNMetaUtils::GetData(Struct, Name ## Key); } \
		static FString Get ## Name##Unsafe(UStruct* Struct) { return FNMetaUtils::GetDataUnsafe(Struct, Name ## Key); } \
		static bool TryGet## Name(UStruct* Struct, FString& OutValue) { return FNMetaUtils::TryGetData(Struct, Name ## Key, OutValue);  }; \
\
		static bool Has ## Name(UClass* Class) { return FNMetaUtils::HasKey(Class, Name ## Key); } \
		static FString Get ## Name(UClass* Class) { return FNMetaUtils::GetData(Class, Name ## Key); } \
		static FString Get ## Name##Unsafe(UClass* Class) { return FNMetaUtils::GetDataUnsafe(Class, Name ## Key); } \
		static bool TryGet## Name(UClass* Class, FString& OutValue) { return FNMetaUtils::TryGetData(Class, Name ## Key, OutValue);  }; \
	private: \
		static FName Name ## Key;

/**
 * A set of utility functions related to accessing meta-information from graphs, nodes, classes, etc. accessed natively.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/editor-types/meta-utils/">FNMetaUtils</a>
 */
class NEXUSCOREEDITOR_API FNMetaUtils
{
public:
	static FString EmptyString;

	/**
	 * Does the target node have any meta-data for the defined key?
	 * @param Node Target UEdGraphNode to evaluate for meta-details.
	 * @param Key The key to look for in the meta-data.
	 * @return true/false if the key was found.
	 */
	static bool HasKey(UEdGraphNode* Node, const FName& Key);
	
	/**
	 * Does the target struct have any meta-data for the defined key?
	 * @param ScriptStruct Target UScriptStruct to evaluate for meta-details.
	 * @param Key The key to look for in the meta-data.
	 * @return true/false if the key was found.
	 */	
	static bool HasKey(const UScriptStruct* ScriptStruct, const FName& Key);

	/**
	 * Does the target struct have any meta-data for the defined key?
	 * @param Struct Target UStruct to evaluate for meta-details.
	 * @param Key The key to look for in the meta-data.
	 * @return true/false if the key was found.
	 */	
	static bool HasKey(const UStruct* Struct, const FName& Key);

	/**
	 * Does the target class have any meta-data for the defined key?
	 * @param Class Target UClass to evaluate for meta-details.
	 * @param Key The key to look for in the meta-data.
	 * @return true/false if the key was found.
	 */
	static bool HasKey(const UClass* Class, const FName& Key);

	/**
	 * Safely attempt to get the data for a specific key, returning an empty string if not found.
	 * @param Node Target UEdGraphNode to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @return The meta-data for the provided key.
	 */
	static FString GetData(UEdGraphNode* Node, const FName& Key);

	/**
	 * Safely attempt to get the data for a specific key, returning an empty string if not found.
	 * @param ScriptStruct Target UScriptStruct to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @return The meta-data for the provided key.
	 */	
	static FString GetData(const UScriptStruct* ScriptStruct, const FName& Key);

	/**
	 * Safely attempt to get the data for a specific key, returning an empty string if not found.
	 * @param Struct Target UStruct to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @return The meta-data for the provided key.
	 */		
	static FString GetData(const UStruct* Struct, const FName& Key);

	/**
	 * Safely attempt to get the data for a specific key, returning an empty string if not found.
	 * @param Class Target UClass to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @return The meta-data for the provided key.
	 */		
	static FString GetData(const UClass* Class, const FName& Key);
	
	/**
	 * Attempt to get the data for a specific key.
	 * @param Node Target UEdGraphNode to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @param OutValue If data is found, set it here.
	 * @return true/false if an entry is found in meta-data.
	 */
	static bool TryGetData(UEdGraphNode* Node, const FName& Key, FString& OutValue);

	/**
	 * Attempt to get the data for a specific key.
	 * @param ScriptStruct Target UScriptStruct to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @param OutValue If data is found, set it here.
	 * @return true/false if an entry is found in meta-data.
	 */
	static bool TryGetData(const UScriptStruct* ScriptStruct, const FName& Key, FString& OutValue);

	/**
	 * Attempt to get the data for a specific key.
	 * @param Struct Target UStruct to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @param OutValue If data is found, set it here.
	 * @return true/false if an entry is found in meta-data.
	 */
	static bool TryGetData(const UStruct* Struct, const FName& Key, FString& OutValue);

	/**
	 * Attempt to get the data for a specific key.
	 * @param Class Target UClass to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.
	 * @param OutValue If data is found, set it here.
	 * @return true/false if an entry is found in meta-data.
	 */
	static bool TryGetData(const UClass* Class, const FName& Key, FString& OutValue);

	/**
	 * Get the data for a specific key without any safety checks.
	 * @remark No checks for existence.
	 * @param Node Target UEdGraphNode to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.	 
	 * @return The meta-data for the provided key.
	 */
	static FString GetDataUnsafe(UEdGraphNode* Node, const FName& Key);

	/**
	 * Get the data for a specific key without any safety checks.
	 * @remark No checks for existence.
	 * @param ScriptStruct Target UScriptStruct to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.	 
	 * @return The meta-data for the provided key.
	 */
	static FString GetDataUnsafe(const UScriptStruct* ScriptStruct, const FName& Key);

	/**
	 * Get the data for a specific key without any safety checks.
	 * @remark No checks for existence.
	 * @param Struct Target UStruct to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.	 
	 * @return The meta-data for the provided key.
	 */
	static FString GetDataUnsafe(const UStruct* Struct, const FName& Key);

	/**
	 * Get the data for a specific key without any safety checks.
	 * @remark No checks for existence.
	 * @param Class Target UClass to evaluate for data based on the key.
	 * @param Key The key to look for in the meta-data.	 
	 * @return The meta-data for the provided key.
	 */
	static FString GetDataUnsafe(const UClass* Class, const FName& Key);

	N_IMPLEMENT_META_TYPE_HEADER(ExternalDocumentation)
};
