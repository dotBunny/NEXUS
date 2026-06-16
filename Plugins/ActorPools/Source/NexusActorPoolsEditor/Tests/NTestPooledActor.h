// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_EDITORONLY_DATA

#include "CoreMinimal.h"
#include "NPooledActorBase.h"
#include "NTestPooledActor.generated.h"

/** Minimal concrete ANPooledActorBase used by the Actor Pools editor tests; just a bare scene-component root. */
UCLASS(ClassGroup = "NEXUS", Hidden, HideDropdown, DisplayName = "NEXUS | Test Pooled Actor")
class ANTestPooledActor : public ANPooledActorBase
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

#endif // WITH_EDITORONLY_DATA
