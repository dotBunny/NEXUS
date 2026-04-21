// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Constants shared across the NexusMultiplayer runtime module.
 */
namespace NEXUS::Multiplayer
{
	/** Sentinel value returned by ping queries when a valid ping isn't available. */
	constexpr float InvalidPing = -1.f;
}

/** Log category used by all NexusMultiplayer logging. */
NEXUSMULTIPLAYER_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusMultiplayer, Log, All);