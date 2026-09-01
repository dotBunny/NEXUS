// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Macros/NActorMacros.h"

#include "NTargetPointComponent.generated.h"

/**
 * A bare positional marker placed on an actor for PCG graphs to read back as points.
 *
 * PCG's built-in component parsing only understands splines, shapes, primitives and virtual textures,
 * so a plain scene component yields no data through Get Actor Data. Pair this with the Get Component Points
 * element, which reads these directly and emits one point per marker.
 *
 * @note An editor-only billboard is what carries the marker's presence in the world; FNTargetPointComponentVisualizer
 *       layers the authoring detail — orientation, scale, tag labels — on top of it once the marker is selected.
 *       The sprite is not decoration: a Blueprint editor viewport runs component visualizers only for components
 *       selected in the Components panel (FSCSEditorViewportClient::Draw), never for the rest of the preview
 *       actor the way a level viewport does, so a marker drawn purely by its visualizer is invisible there and,
 *       having nothing else in the world, offers no click target to become selected with. The sprite breaks that
 *       loop: clicking it resolves up the attachment chain to this component's tree node, which is what then
 *       brings the visualizer in.
 * @note A billboard is the only primitive that may hang off a marker. Get Actor Data walks every component on the
 *       actor and turns each UPrimitiveComponent it does not recognise into UPCGPrimitiveData, with exactly one
 *       carve-out — UBillboardComponent (FPCGGetDataFunctionRegistry::DefaultDataFromComponent). An arrow, a
 *       shape, or a mesh used as the icon would feed the marker's own decoration back into the graph.
 * @note The icon comes from N_WORLD_ICON_ON_REGISTER, the register-time macro, and not from the constructor form
 *       the NEXUS actors use — a marker is a Blueprint-spawnable component, and a default subobject of a component
 *       carries a serialized AttachParent that points at the component template rather than the instance. See the
 *       warning on N_WORLD_ICON_SCENE_COMPONENT.
 * @see <a href="https://nexus-framework.com/docs/core/types/pcg/target-point-component/">UNTargetPointComponent</a>
 */
// Tags is deliberately left visible: Get Component Points filters on this component's own tags, so hiding
// the category the way the other NEXUS marker components do would put that filter out of reach.
UCLASS(ClassGroup="NEXUS", DisplayName = "NEXUS | Target Point", meta = (BlueprintSpawnableComponent),
	HideCategories=(Activation, AssetUserData, Cooking, Navigation, HLOD, LOD, Rendering, Collision, Physics))
class NEXUSCORE_API UNTargetPointComponent : public USceneComponent
{
	GENERATED_BODY()

	UNTargetPointComponent(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	//~USceneComponent
	virtual void OnRegister() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//End USceneComponent
#endif // WITH_EDITOR

	N_WORLD_ICON_COMPONENT_HEADER()
};
