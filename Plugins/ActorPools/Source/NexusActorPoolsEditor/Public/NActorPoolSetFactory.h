// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "NActorPoolSet.h"
#include "Macros/NEditorAssetMacros.h"
#include "NActorPoolSetFactory.generated.h"

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

