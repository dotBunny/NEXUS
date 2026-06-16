// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "NPickerMinimal.h"
#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

#define N_PICKER_PARAMS_WORLD_SAFETY \
	UWorld* CachedWorld = Params.CachedWorld.Get(); \

// Resolves the shared, framework-wide non-deterministic FRandomStream into a local reference named Random.
// @note Not thread-safe; FNRandom::GetNonDeterministic() hands back a single shared FRandomStream that every
//       picker's Random() path mutates. Only use from the Game-thread.
#define N_PICKER_RANDOM_NONDETERMINISTIC FRandomStream& Random = FNRandom::GetNonDeterministic();

// Point generation and projection share a single home in FNPickerProjection::Emit (NPickerProjection.h);
// the per-mode trace/navmesh loops that used to be unrolled in every picker now live there.

/**
 * Shared picker configuration defaults used by all pickers when resolving a generated point onto geometry or navmesh via ENPickerProjectionMode.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	/** Collision query parameters used by the trace-based projection path. 
	 * @note Should only be accessed from the Game-thread.
	 */
	static FCollisionQueryParams CollisionQueryParams;
	/** Query extent used when projecting onto the navmesh; adjust if your navmesh tolerance needs widening. 
	 * @note Should only be accessed from the Game-thread.
	 */
	static FVector NavQueryExtent;
	/** Nav agent properties used when resolving a navmesh location for a generated point.
	 * @note Should only be accessed from the Game-thread. 
	 */
	static FNavAgentProperties NavAgentProperties;
};