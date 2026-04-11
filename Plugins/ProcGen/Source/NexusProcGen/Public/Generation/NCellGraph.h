// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellGraphNode.h"

class NEXUSPROCGEN_API FNCellGraph
{
public:
	explicit FNCellGraph(FNCellGraphNode* RootNodePtr);
	~FNCellGraph();

	const TArray<FNCellGraphNode*>& GetNodes() const { return Nodes; }
	TArray<FNCellGraphNode*>& GetMutableNodes() { return Nodes; }
private:
	FNCellGraphNode* RootNode = nullptr;
	TArray<FNCellGraphNode*> Nodes;
};
