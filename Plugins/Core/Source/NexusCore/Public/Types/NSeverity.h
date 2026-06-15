// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Ordered diagnostic severity level, ascending from purely informational to unrecoverable.
 *
 * The numeric ordering is meaningful: callers may threshold on it (e.g. "Warning and above") to filter output.
 */
UENUM(BlueprintType)
enum class ENSeverity : uint8
{
	/** Low-level informational detail; safe to ignore in normal operation. */
	Info		= 0,
	/** General notice worth surfacing but not indicative of a problem. */
	Message		= 1,
	/** A potential problem that does not stop execution. */
	Warning		= 2,
	/** A failure that prevented an operation from completing. */
	Error		= 3,
	/** An unrecoverable failure; continuing is unsafe. */
	Fatal		= 4
};