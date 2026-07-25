// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// NOTE: Deliberately not wrapped in a preprocessor guard. UHT emits this class into the module's
// unscoped registration table (sizeof(ANTestPooledActor) and friends), so any #if around a reflected
// type breaks non-editor targets where that define is 0. Guard test bodies with WITH_TESTS instead.

#include "CoreMinimal.h"
#include "NPooledActorBase.h"
#include "NTestPooledActor.generated.h"

/**
 * Minimal concrete ANPooledActorBase used by the Actor Pools tests; just a bare scene-component root.
 * Exported so NexusActorPoolsEditorTests can share the single definition — a second copy would collide
 * in the UObject registry.
 */
UCLASS(ClassGroup = "NEXUS", Hidden, HideDropdown, DisplayName = "NEXUS | Test Pooled Actor")
class NEXUSACTORPOOLSTESTS_API ANTestPooledActor : public ANPooledActorBase
{
	GENERATED_BODY()

	ANTestPooledActor()
	{
		SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
		RootComponent = SceneComponent;
	}

protected:
	/** Bare root component giving the test actor a valid root transform. */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USceneComponent> SceneComponent;
};
