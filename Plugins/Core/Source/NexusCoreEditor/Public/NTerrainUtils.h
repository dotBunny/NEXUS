// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCoreEditorMinimal.h"

class ULevel;

/**
 * Editor-time utilities for working with a level's terrain while it is still being built.
 *
 * Mesh Partition lands an authored terrain's sections across many frames, and exposes no barrier covering the whole
 * pipeline. Anything that measures a level — bounds, a hull, a voxel field, a collision cache — therefore has to know
 * whether it is looking at finished geometry or at a build in progress, and these are how it finds out. Both work off
 * the terrain classification in FNActorUtils, so they cover every representation that recognizes.
 *
 * @see <a href="https://nexus-framework.com/docs/core/editor-types/terrain-utils/">FNTerrainUtils</a>
 */
class NEXUSCOREEDITOR_API FNTerrainUtils
{
public:
	/**
	 * Summarize a level's terrain geometry so a build still landing sections can be told from a finished one.
	 * @param InLevel Level to summarize. A null level, or one with no terrain, returns 0.
	 * @return A hash that changes whenever the terrain geometry does.
	 * @note Quantized to whole units, so float jitter in an otherwise settled bound does not read as movement.
	 */
	static uint32 ComputeFingerprint(const ULevel* InLevel);

	/**
	 * Block until the level's terrain geometry stops changing, pumping the systems a terrain build depends on.
	 *
	 * @param InLevel Level whose terrain to wait on. A null level returns true immediately.
	 * @param TimeoutSeconds Give up after this long and report failure rather than blocking the editor forever.
	 * @return true when the terrain settled; false on timeout.
	 * @remark **Must not be called from inside a save.** It ticks editor objects, and a Mesh Partition tick spawns and
	 *         destroys section actors — which during UEditorEngine::SavePackage would mutate the world mid-write. Call
	 *         it where a save or a calculation is *initiated*, ahead of the engine's save machinery.
	 * @note Does not pump Slate or a full engine tick, so the editor stays visually frozen for the duration; the slow
	 *       task it opens is what tells the user why.
	 */
	static bool WaitForSettle(const ULevel* InLevel, double TimeoutSeconds = NEXUS::CoreEditor::Terrain::DefaultSettleTimeoutSeconds);
};
