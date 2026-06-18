// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CollisionQueryParams.h"
#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "NPickerSettings.generated.h"

/**
 * Project-wide configuration for the NexusPicker plugin.
 *
 * Controls how a generated point is resolved onto geometry (trace) or the navmesh when a picker's
 * ENPickerProjectionMode is set. Exposed under Project Settings > NEXUS > Picker; stored in
 * DefaultNexusGame.ini. Replaces the previous FNPickerUtils static configuration members so projection
 * defaults are discoverable, editor-editable, and persisted per-project rather than reset every launch.
 * @note Resolved on the Game-thread during point generation; see FNPickerProjection::Emit.
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/project-settings/">UNPickerSettings</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Picker Settings", Config=NexusGame, defaultconfig)
class NEXUSPICKER_API UNPickerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_SETTINGS_BASE(UNPickerSettings, "Picker", "Settings related to resolving generated points onto geometry or the navmesh.")

public:
	/** Trace against complex (per-polygon) collision instead of simple collision when projecting points onto geometry. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Projection|Trace", DisplayName = "Trace Complex",
		meta=(ToolTip="Trace against complex (per-polygon) collision instead of simple collision when projecting points onto geometry."))
	bool bTraceComplex = false;

	/** Half-extent of the box searched when projecting a generated point onto the navmesh; widen if points fail to resolve. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Projection|NavMesh", DisplayName = "Nav Query Extent",
		meta=(ToolTip="Half-extent of the box searched when projecting a generated point onto the navmesh. Widen if points fail to resolve."))
	FVector NavQueryExtent = FVector(1500.0, 1500.0, 1500.0);

	/** Radius of the navigation agent used to resolve a navmesh location for a generated point. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Projection|NavMesh", DisplayName = "Nav Agent Radius",
		meta=(ToolTip="Radius of the navigation agent used to resolve a navmesh location for a generated point.", ClampMin="0.0", UIMin="0.0", Units="cm"))
	float NavAgentRadius = 42.0f;

	/** Height of the navigation agent used to resolve a navmesh location for a generated point. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Projection|NavMesh", DisplayName = "Nav Agent Height",
		meta=(ToolTip="Height of the navigation agent used to resolve a navmesh location for a generated point.", ClampMin="0.0", UIMin="0.0", Units="cm"))
	float NavAgentHeight = 192.0f;

	/**
	 * Builds the FCollisionQueryParams used by the trace-based projection path from the configured values.
	 * @return Collision query parameters tagged "NEXUS" with the configured complex-trace preference.
	 * @note Game-thread only; build once per generation call rather than per generated point.
	 */
	FCollisionQueryParams MakeCollisionQueryParams() const
	{
		return FCollisionQueryParams(FName(TEXT("NEXUS")), bTraceComplex);
	}
};
