// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraphNullNode.h"

#include "NProcGenMinimal.h"

void FNProcGenGraphNullNode::Link(FNProcGenGraphNode* Node)
{
	if (Linked != nullptr)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("FNProcGenGraphNullNode already linked."))
		return;
	}
	Linked = Node;
}

void FNProcGenGraphNullNode::Unlink()
{
	if (Linked == nullptr)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("FNProcGenGraphNullNode not linked."))
		return;
	}
	Linked = nullptr;
}