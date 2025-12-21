// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_UNREGISTER_PLACEABLE_ACTORS(PlacementActors) \
if (IPlacementModeModule::IsAvailable()) \
{ \
	IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get(); \
	for (TOptional<FPlacementModeID>& PlaceActor : PlacementActors) \
	{ \
		if (PlaceActor.IsSet()) \
		{ \
			PlacementModeModule.UnregisterPlaceableItem(*PlaceActor); \
		} \
	} \
} \
PlacementActors.Empty();