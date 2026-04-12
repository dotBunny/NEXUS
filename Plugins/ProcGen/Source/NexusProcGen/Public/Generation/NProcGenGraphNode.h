// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

enum class ENProcGenGraphNodeType
{
	Bone,
	Cell,
	Null
};

class NEXUSPROCGEN_API FNProcGenGraphNode
{
public:
	FNProcGenGraphNode(const FVector& Position, const FRotator& Rotation);
	
	virtual ~FNProcGenGraphNode() = default;
	virtual ENProcGenGraphNodeType GetNodeType() const = 0;

	FVector GetWorldPosition() const { return WorldPosition; }
	FRotator GetWorldRotation() const { return WorldRotation; }
	
	
protected:
	void SetWorldPosition(const FVector& Position);
	void SetWorldRotation(const FRotator& Rotation);
	
	
private:
	FVector WorldPosition;
	FRotator WorldRotation;
};