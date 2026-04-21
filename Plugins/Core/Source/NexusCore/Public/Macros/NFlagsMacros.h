// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

/**
 * Bitwise-flag helpers used throughout NEXUS to query and mutate integer flag fields.
 *
 * The macros cover both the "has all bits set in Mask" idiom and add/remove operations, plus
 * UINT8 overloads for enum-class flag types that require explicit casting.
 */

/** Evaluates to true when every bit in Mask is also set in Flags. */
#define N_FLAGS_HAS(Flags, Mask) \
((Flags & Mask) == Mask)

/** N_FLAGS_HAS variant that casts Mask to uint8 first (useful for enum-class flag types). */
#define N_FLAGS_HAS_UINT8(Flags, Mask) \
((Flags & static_cast<uint8>(Mask)) == static_cast<uint8>(Mask))

/** N_FLAGS_HAS variant that casts both Flags and Mask to uint8. */
#define N_FLAGS_UINT8_HAS_UINT8(Flags, Mask) \
((static_cast<uint8>(Flags) & static_cast<uint8>(Mask)) == static_cast<uint8>(Mask))

/** Evaluates to true when every bit in Mask is set in Flags (alias of N_FLAGS_HAS, clearer at call site). */
#define N_FLAGS_HAS_ALL_OF(Flags, Mask) \
((Flags & Mask) == Mask)

/** Evaluates to true when at least one bit in Mask is set in Flags. */
#define N_FLAGS_HAS_ANY_OF(Flags, Mask) \
((Flags & Mask) != 0)

/** Evaluates to true when no bits in Mask are set in Flags. */
#define N_FLAGS_HAS_NONE_OF(Flags, Mask) \
((Flags & Mask) == 0)

/** Sets every bit in Mask on Flags in place. */
#define N_FLAGS_ADD(Flags, Mask) \
Flags |= Mask

/** uint8 cast variant of N_FLAGS_ADD, for enum-class flag types. */
#define N_FLAGS_ADD_UINT8(Flags, Mask) \
Flags |= static_cast<uint8>(Mask);

/** Clears every bit in Mask on Flags in place. */
#define N_FLAGS_REMOVE(Flags, Mask) \
Flags &= ~Mask

/** uint8 cast variant of N_FLAGS_REMOVE, for enum-class flag types. */
#define N_FLAGS_REMOVE_UINT8(Flags, Mask) \
Flags &= ~static_cast<uint8>(Mask)