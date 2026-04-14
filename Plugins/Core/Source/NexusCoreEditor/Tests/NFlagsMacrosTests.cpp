// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NFlagsMacros.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(NFlagsMacrosTests_Has_BitsSet, "NEXUS::UnitTests::NCore::NFlagsMacros::Has_BitsSet", N_TEST_CONTEXT_ANYWHERE)
{
	const uint32 Flags = 0b1010u;
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS should return true when all mask bits are set"), N_FLAGS_HAS(Flags, 0b1010u));
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS should return true for a subset of set bits"), N_FLAGS_HAS(Flags, 0b0010u));
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS should return true for single matching bit"), N_FLAGS_HAS(Flags, 0b1000u));
}

N_TEST_HIGH(NFlagsMacrosTests_Has_BitsNotSet, "NEXUS::UnitTests::NCore::NFlagsMacros::Has_BitsNotSet", N_TEST_CONTEXT_ANYWHERE)
{
	const uint32 Flags = 0b1010u;
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS should return false when mask bits are not set"), N_FLAGS_HAS(Flags, 0b0001u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS should return false when mask is a superset"), N_FLAGS_HAS(Flags, 0b1111u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS should return false when no flags are set"), N_FLAGS_HAS(0u, 0b1010u));
}

N_TEST_HIGH(NFlagsMacrosTests_HasAllOf, "NEXUS::UnitTests::NCore::NFlagsMacros::HasAllOf", N_TEST_CONTEXT_ANYWHERE)
{
	const uint32 Flags = 0b1010u;
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS_ALL_OF should return true when all mask bits present"), N_FLAGS_HAS_ALL_OF(Flags, 0b1010u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS_ALL_OF should return false if any mask bit missing"), N_FLAGS_HAS_ALL_OF(Flags, 0b1011u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS_ALL_OF should return false if zero flags"), N_FLAGS_HAS_ALL_OF(0u, 0b1010u));
}

N_TEST_HIGH(NFlagsMacrosTests_HasAnyOf, "NEXUS::UnitTests::NCore::NFlagsMacros::HasAnyOf", N_TEST_CONTEXT_ANYWHERE)
{
	const uint32 Flags = 0b1010u;
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS_ANY_OF should return true if first bit matches"), N_FLAGS_HAS_ANY_OF(Flags, 0b1000u));
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS_ANY_OF should return true if second bit matches"), N_FLAGS_HAS_ANY_OF(Flags, 0b0010u));
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS_ANY_OF should return true if multiple bits match"), N_FLAGS_HAS_ANY_OF(Flags, 0b1110u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS_ANY_OF should return false if no bits match"), N_FLAGS_HAS_ANY_OF(Flags, 0b0101u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS_ANY_OF with zero mask should return false"), N_FLAGS_HAS_ANY_OF(Flags, 0u));
}

N_TEST_HIGH(NFlagsMacrosTests_HasNoneOf, "NEXUS::UnitTests::NCore::NFlagsMacros::HasNoneOf", N_TEST_CONTEXT_ANYWHERE)
{
	const uint32 Flags = 0b1010u;
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS_NONE_OF should return true if no bits match"), N_FLAGS_HAS_NONE_OF(Flags, 0b0101u));
	CHECK_MESSAGE(TEXT("N_FLAGS_HAS_NONE_OF should return true with zero mask"), N_FLAGS_HAS_NONE_OF(Flags, 0u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS_NONE_OF should return false if one bit matches"), N_FLAGS_HAS_NONE_OF(Flags, 0b0010u));
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_HAS_NONE_OF should return false if all bits match"), N_FLAGS_HAS_NONE_OF(Flags, 0b1010u));
}

N_TEST_HIGH(NFlagsMacrosTests_Add, "NEXUS::UnitTests::NCore::NFlagsMacros::Add", N_TEST_CONTEXT_ANYWHERE)
{
	uint32 Flags = 0u;

	N_FLAGS_ADD(Flags, 0b0001u);
	CHECK_MESSAGE(TEXT("N_FLAGS_ADD should set the specified bit"), N_FLAGS_HAS(Flags, 0b0001u));

	N_FLAGS_ADD(Flags, 0b1000u);
	CHECK_MESSAGE(TEXT("N_FLAGS_ADD second bit should be set"), N_FLAGS_HAS(Flags, 0b1000u));
	CHECK_MESSAGE(TEXT("N_FLAGS_ADD should not affect previously set bits"), N_FLAGS_HAS(Flags, 0b0001u));

	// Adding an already-set bit is idempotent
	N_FLAGS_ADD(Flags, 0b0001u);
	CHECK_MESSAGE(TEXT("N_FLAGS_ADD on already-set bit should be idempotent"), N_FLAGS_HAS(Flags, 0b0001u));
}

N_TEST_HIGH(NFlagsMacrosTests_Remove, "NEXUS::UnitTests::NCore::NFlagsMacros::Remove", N_TEST_CONTEXT_ANYWHERE)
{
	uint32 Flags = 0b1010u;

	N_FLAGS_REMOVE(Flags, 0b0010u);
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_REMOVE should clear the specified bit"), N_FLAGS_HAS(Flags, 0b0010u));
	CHECK_MESSAGE(TEXT("N_FLAGS_REMOVE should not affect other bits"), N_FLAGS_HAS(Flags, 0b1000u));

	// Removing an already-clear bit is idempotent
	N_FLAGS_REMOVE(Flags, 0b0010u);
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_REMOVE on already-cleared bit should be idempotent"), N_FLAGS_HAS(Flags, 0b0010u));

	N_FLAGS_REMOVE(Flags, 0b1000u);
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_REMOVE should clear the last bit"), N_FLAGS_HAS(Flags, 0b1000u));
	CHECK_MESSAGE(TEXT("All bits removed should equal zero"), Flags == 0u);
}

N_TEST_HIGH(NFlagsMacrosTests_AddRemove_UInt8, "NEXUS::UnitTests::NCore::NFlagsMacros::AddRemove_UInt8", N_TEST_CONTEXT_ANYWHERE)
{
	uint8 Flags = 0u;

	N_FLAGS_ADD_UINT8(Flags, 0b00000100u);
	CHECK_MESSAGE(TEXT("N_FLAGS_ADD_UINT8 should set the bit"), N_FLAGS_HAS_UINT8(Flags, 0b00000100u));

	N_FLAGS_REMOVE_UINT8(Flags, 0b00000100u);
	CHECK_FALSE_MESSAGE(TEXT("N_FLAGS_REMOVE_UINT8 should clear the bit"), N_FLAGS_HAS_UINT8(Flags, 0b00000100u));
}

#endif //WITH_TESTS
