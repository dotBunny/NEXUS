// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NPooledActor.h"
#include "NTestPooledActor.generated.h"

UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Test Pooled Actor")
class NEXUSACTORPOOLSEDITOR_API ANTestPooledActor : public ANPooledActor
{
	GENERATED_BODY()

	ANTestPooledActor()
	{
		SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
		RootComponent = SceneComponent;
	}

protected:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USceneComponent> SceneComponent;
};
