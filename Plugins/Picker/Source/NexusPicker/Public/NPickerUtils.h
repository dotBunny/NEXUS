// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "NPickerMinimal.h"
#include "CoreMinimal.h"

#define N_PICKER_PARAMS_WORLD_SAFETY \
	UWorld* CachedWorld = Params.CachedWorld.Get(); \

// Resolves the shared, framework-wide non-deterministic FRandomStream into a local reference named Random.
// @note Not thread-safe; FNRandom::GetNonDeterministic() hands back a single shared FRandomStream that every
//       picker's Random() path mutates. Only use from the Game-thread.
#define N_PICKER_RANDOM_NONDETERMINISTIC FRandomStream& Random = FNRandom::GetNonDeterministic();

// Point generation and projection share a single home in FNPickerProjection::Emit (NPickerProjection.h);
// the per-mode trace/navmesh loops that used to be unrolled in every picker now live there.
//
// Projection configuration (trace complexity, navmesh query extent, nav agent dimensions) lives in
// UNPickerSettings (NPickerSettings.h), surfaced under Project Settings > NEXUS > Picker.