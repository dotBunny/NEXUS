// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"


class UNBoneComponent;
class UNWorldAssemblySettings;

/**
 * Component visualizer for UNBoneComponent — renders the bone's socket, reach, and mode-specific
 * widgets in the level viewport so authors can reason about junction anchors without selection.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-types/visualizers/bone-component-visualizer/">FNBoneComponentVisualizer</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNBoneComponentVisualizer final : public FComponentVisualizer
{
public:
	//~FComponentVisualizer
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//End FComponentVisualizer

private:
	/** One bone's memoized penetration plus the inputs it was computed for; reused across frames while those are unchanged. */
	struct FCachedPenetration
	{
		FTransform KeyTransform = FTransform::Identity;
		FIntVector2 KeySocketSize = FIntVector2(0, 0);
		FVector2D KeySettingSocketSize = FVector2D::ZeroVector;
		/** The bone's world's published-results generation this was computed against; a mismatch invalidates the entry. */
		uint32 KeyResultsGeneration = 0;
		float Penetration = 0.f;
	};

	/**
	 * Returns the socket-corner penetration of BoneComponent into world collision, recomputing (against the shared,
	 * BVH-accelerated world-collision mesh) only when the world geometry or the bone's own inputs have changed.
	 * DrawVisualization runs on every viewport redraw, so this memo is what keeps an idle viewport from re-sweeping
	 * the whole world mesh per bone per frame.
	 */
	static float GetCachedWorldPenetration(const UNBoneComponent* BoneComponent, const UNWorldAssemblySettings* Settings);

	/**
	 * Per-bone penetration memo, shared across every world this visualizer draws for (level viewport + Blueprint-editor
	 * preview scenes). Each entry records the world generation it was built against rather than relying on a single
	 * global counter, so a rebuild in one world never invalidates bones in another. Entries for destroyed bones are
	 * pruned lazily on the (rare) recompute path via their weak key.
	 */
	static TMap<TWeakObjectPtr<const UNBoneComponent>, FCachedPenetration> PenetrationCache;
};
