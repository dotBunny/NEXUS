// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_FLAGS_HAS(Flags, Mask) \
((Flags & Mask) == Mask)

#define N_FLAGS_HAS_UINT8(Flags, Mask) \
((Flags & static_cast<uint8>(Mask)) == static_cast<uint8>(Mask))

#define N_FLAGS_UINT8_HAS_UINT8(Flags, Mask) \
((static_cast<uint8>(Flags) & static_cast<uint8>(Mask)) == static_cast<uint8>(Mask))

#define N_FLAGS_HAS_ALL_OF(Flags, Mask) \
((Flags & Mask) == Mask)

#define N_FLAGS_HAS_ANY_OF(Flags, Mask) \
((Flags & Mask) != 0)

#define N_FLAGS_HAS_NONE_OF(Flags, Mask) \
((Flags & Mask) == 0)

#define N_FLAGS_ADD(Flags, Mask) \
Flags |= Mask

#define N_FLAGS_ADD_UINT8(Flags, Mask) \
Flags |= static_cast<uint8>(Mask);

#define N_FLAGS_REMOVE(Flags, Mask) \
Flags &= ~Mask

#define N_FLAGS_REMOVE_UINT8(Flags, Mask) \
Flags &= ~static_cast<uint8>(Mask)