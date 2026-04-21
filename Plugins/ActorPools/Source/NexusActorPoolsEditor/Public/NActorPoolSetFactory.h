// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "NActorPoolSet.h"
#include "Macros/NEditorAssetMacros.h"
#include "NActorPoolSetFactory.generated.h"

/**
 * Asset factory that creates new UNActorPoolSet assets from the Content Browser's "Create Asset" menu.
 */
UCLASS(MinimalAPI, HideCategories = Object)
class UNActorPoolSetFactory : public UFactory
{
	GENERATED_BODY()

	explicit UNActorPoolSetFactory(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SupportedClass = UNActorPoolSet::StaticClass();
		bCreateNew = true;
		bEditorImport = false;
		bEditAfterNew = false;
	}
	N_IMPLEMENT_ASSET_FACTORY(UNActorPoolSet)
};

