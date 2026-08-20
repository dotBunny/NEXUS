// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "NTargetPointComponent.generated.h"

/**
 * A bare positional marker placed on an actor for PCG graphs to read back as points.
 *
 * PCG's built-in component parsing only understands splines, shapes, primitives and virtual textures,
 * so a plain scene component yields no data through Get Actor Data. Pair this with the Get Target Points
 * element, which reads these directly and emits one point per marker.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/target-point-component/">UNTargetPointComponent</a>
 */
// Tags is deliberately left visible: Get Target Points filters on this component's own tags, so hiding
// the category the way the other NEXUS marker components do would put that filter out of reach.
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Target Point", meta = (BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSCORE_API UNTargetPointComponent : public USceneComponent
{
	GENERATED_BODY()
};
