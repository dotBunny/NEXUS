// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraphBoneNode.h"

#include "NWorldAssemblyMinimal.h"

void FNAssemblyGraphBoneNode::Link(FNAssemblyGraphNode* Node)
{
	if (Linked != nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("FNWorldAssemblyGraphBoneNode already linked."));
		return;
	}
	Linked = Node;
}

void FNAssemblyGraphBoneNode::Unlink()
{
	if (Linked == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("FNWorldAssemblyGraphBoneNode not linked."));
		return;
	}
	Linked = nullptr;
}
