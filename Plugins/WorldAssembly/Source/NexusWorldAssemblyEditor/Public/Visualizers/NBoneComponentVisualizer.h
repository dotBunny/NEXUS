// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "ComponentVisualizer.h"


class UNBoneComponent;
class UNWorldAssemblySettings;

/**
 * Component visualizer for UNBoneComponent — renders the bone's socket, reach, and mode-specific
 * widgets in the level viewport so authors can reason about junction anchors without selection.
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
		float Penetration = 0.f;
	};

	/**
	 * Returns the socket-corner penetration of BoneComponent into world collision, recomputing (against the shared,
	 * BVH-accelerated world-collision mesh) only when the world geometry or the bone's own inputs have changed.
	 * DrawVisualization runs on every viewport redraw, so this memo is what keeps an idle viewport from re-sweeping
	 * the whole world mesh per bone per frame.
	 */
	static float GetCachedWorldPenetration(const UNBoneComponent* BoneComponent, const UNWorldAssemblySettings* Settings);

	/** Collision-cache generation the memo was last valid for; a mismatch clears the whole map. */
	static uint32 CachedGeneration;

	/** Per-bone penetration memo. Weak keys make entries for destroyed bones harmless until the next generation clear. */
	static TMap<TWeakObjectPtr<const UNBoneComponent>, FCachedPenetration> PenetrationCache;
};
