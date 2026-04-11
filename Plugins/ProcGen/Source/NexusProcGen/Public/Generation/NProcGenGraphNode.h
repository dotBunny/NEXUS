// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

enum class ENProcGenGraphNodeType
{
	Bone,
	Cell
};

class NEXUSPROCGEN_API FNProcGenGraphNode
{
public:
	
	virtual ~FNProcGenGraphNode() = default;
	virtual ENProcGenGraphNodeType GetNodeType() const = 0;

	FVector GetWorldPosition() const { return WorldPosition; }
	FRotator GetWorldRotation() const { return WorldRotation; }

protected:
	FVector WorldPosition;
	FRotator WorldRotation;
	
};