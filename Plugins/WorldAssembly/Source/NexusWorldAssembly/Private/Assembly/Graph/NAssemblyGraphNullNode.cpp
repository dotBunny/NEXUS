// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Graph/NAssemblyGraphNullNode.h"

#include "NWorldAssemblyMinimal.h"

void FNAssemblyGraphNullNode::Link(FNAssemblyGraphNode* Node)
{
	if (Linked != nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("FNWorldAssemblyGraphNullNode already linked."));
		return;
	}
	Linked = Node;
}

void FNAssemblyGraphNullNode::Unlink()
{
	if (Linked == nullptr)
	{
		UE_LOG(LogNexusWorldAssembly, Error, TEXT("FNWorldAssemblyGraphNullNode not linked."));
		return;
	}
	Linked = nullptr;
}