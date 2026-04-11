// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCell.h"

class NEXUSPROCGEN_API FNCellGraphNode
{
public:
	FNCellGraphNode(const TObjectPtr<UNCell> Template)
	{
		TemplatePtr = Template;	
	}
	FNCellGraphNode(const TObjectPtr<UNCell> Template, const FVector& Position, const FRotator& Rotation)
	{
		WorldPosition = Position;
		WorldRotation = Rotation;
		
		TemplatePtr = Template;
	}
	
	void UpdateWorldPosition(const FVector& Position);
	void UpdateWorldRotation(const FRotator& Rotation);

	FVector GetWorldPosition() const { return WorldPosition; }
	FRotator GetWorldRotation() const { return WorldRotation; }
	TObjectPtr<UNCell> GetTemplate() const { return TemplatePtr; }

private:
	FVector WorldPosition;
	FRotator WorldRotation;
	
	TMap<int32, FNCellGraphNode*> JunctionLinks;
	
	// MAIN-THREAD USE ONLY // ROOT OBJECT NEEDS TO BE UNLOADED LATER
	TObjectPtr<UNCell> TemplatePtr;
};
