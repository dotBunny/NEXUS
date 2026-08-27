// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NValidation.generated.h"

/**
 * State of a baked artifact: whether it exists, and whether it still matches the input it was baked from.
 *
 * Two flags rather than three states, because existence and currency are independent questions and the interesting
 * answer is the combination — Baked alone is a stale bake, which is exactly the case a re-bake decision turns on and
 * the one a tri-state enum would collapse into "not up to date".
 * @note UseEnumValuesAsMaskValuesInEditor makes these literal masks rather than bit indices, so a third flag
 *       continues 4, 8, ... Without it Unreal would read UpToDate = 2 as bit 2 and disagree with any C++ that did not.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/types/validation/">ENBakedValidation</a>
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENBakedValidation : uint8
{
	/** Neither flag set — never baked. Hidden because it is the empty mask, not a state to pick. */
	None = 0 UMETA(Hidden),
	/** The artifact exists. */
	Baked = 1,
	/** The artifact matches the input it was baked from. */
	UpToDate = 2
};
ENUM_CLASS_FLAGS(ENBakedValidation)

/** Fully-valid mask: baked and current. uint8 so it compares directly against a Bitflags UPROPERTY. */
#define D_BAKED_VALIDATION_ALL static_cast<uint8>(ENBakedValidation::Baked) | static_cast<uint8>(ENBakedValidation::UpToDate)

/**
 * State of a generated artifact: whether it exists, and whether it still matches the input it was generated from.
 *
 * The same shape as ENBakedValidation, differing only in what the first flag is called; kept as its own type so a
 * property cannot be handed the wrong vocabulary.
 * @see <a href="https://nexus-framework.com/docs/plugins/core/types/types/validation/">ENGeneratedValidation</a>
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENGeneratedValidation : uint8
{
	/** Neither flag set — never generated. Hidden because it is the empty mask, not a state to pick. */
	None = 0 UMETA(Hidden),
	/** The artifact exists. */
	Generated = 1,
	/** The artifact matches the input it was generated from. */
	UpToDate = 2
};
ENUM_CLASS_FLAGS(ENGeneratedValidation)

/** Fully-valid mask: generated and current. uint8 so it compares directly against a Bitflags UPROPERTY. */
#define D_GENERATED_VALIDATION_ALL static_cast<uint8>(ENGeneratedValidation::Generated) | static_cast<uint8>(ENGeneratedValidation::UpToDate)