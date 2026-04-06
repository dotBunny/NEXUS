// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenOperationBoneContext.h"
#include "Organ/NBoneComponent.h"

class UNOrganComponent;

struct NEXUSPROCGEN_API FNProcGenOperationOrganContext
{
	UNOrganComponent* SourceComponent;
	
	TArray<UNOrganComponent*> IntersectComponents;
	TArray<UNOrganComponent*> ContainedComponents;
	
	TArray<FNProcGenOperationBoneContext*> ContainedBones;
};