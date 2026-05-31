// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Unregisters every FPlacementModeID inside the supplied collection and clears it.
 *
 * Use during an editor module's ShutdownModule to undo RegisterPlaceableItem calls made during StartupModule.
 *
 * @param PlacementActors A container of TOptional<FPlacementModeID> captured at registration time.
 */
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