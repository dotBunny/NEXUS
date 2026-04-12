// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenGraphBoneNode.h"

#include "NProcGenMinimal.h"

void FNProcGenGraphBoneNode::Link(FNProcGenGraphNode* Node)
{
	if (Linked != nullptr)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("FNProcGenGraphBoneNode already linked."))
		return;
	}
	Linked = Node;
}

void FNProcGenGraphBoneNode::Unlink()
{
	if (Linked == nullptr)
	{
		UE_LOG(LogNexusProcGen, Error, TEXT("FNProcGenGraphBoneNode not linked."))
		return;
	}
	Linked = nullptr;
}
