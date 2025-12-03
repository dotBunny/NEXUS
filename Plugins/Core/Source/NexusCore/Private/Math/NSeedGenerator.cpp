// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NSeedGenerator.h"

#include "NCoreMinimal.h"
#include "Math/BigInt.h"
#include "Math/NHashUtils.h"

int FNSeedGenerator::HexToInteger(const TCHAR Char)
{
	switch (Char)
	{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
			return 10;
		case 'B':
			return 11;
		case 'C':
			return 12;
		case 'D':
			return 13;
		case 'E':
			return 14;
		case 'F':
			return 15;
		case 'a':
			return 10;
		case 'b':
			return 11;
		case 'c':
			return 12;
		case 'd':
			return 13;
		case 'e':
			return 14;
		case 'f':
			return 15;
		default:
			return 0;
	}
}

bool FNSeedGenerator::IsValidHexSeed(const FString& InHexSeed)
{
	// Sanitize
	const FString ParsedSeed = SanitizeHexSeed(InHexSeed);
	const int SeedLength = ParsedSeed.Len();

	if (SeedLength == 0 || SeedLength == 1)
	{
		return false;
	}

	// Check that we have an equal number of sets
	if (SeedLength % 2 != 0)
	{
		return false;
	}

	// Success
	return true;
}

uint64 FNSeedGenerator::RandomSeed()
{
	return static_cast<uint64>(FMath::Rand()) << 32 | static_cast<uint64>(FMath::Rand());
}

FString FNSeedGenerator::SanitizeHexSeed(const FString& InHexSeed)
{
	FString Builder;
	Builder.Reserve(16);
	for (int i = 0; i < InHexSeed.Len(); i++)
	{
		if (TChar<TCHAR>::IsHexDigit(InHexSeed[i]))
		{
			Builder.AppendChar(InHexSeed[i]);
		}
	}

	// 0xFFFFFFFFFFFFFFFF
	if (Builder.Len() > 16)
	{
		Builder = Builder.Mid(0, 16);
	}

	return Builder;
}

uint64 FNSeedGenerator::SeedFromText(const FString& InSeed)
{
	const uint64 Seed = FNHashUtils::dbj2(InSeed);
	N_LOG("[FNSeedGenerator::SeedFromText] Created seed (%llu) from string (%s).", Seed, *InSeed);
	return Seed;
}

uint64 FNSeedGenerator::SeedFromHex(const FString& InHexSeed)
{
	// Sanitize the string
	FString ParsedSeed = SanitizeHexSeed(InHexSeed);
	const int SeedLength = ParsedSeed.Len();

	if (SeedLength == 0 || SeedLength == 1)
	{
		N_LOG_WARNING("The parsed (%s) seed (%s) length (%i) was below any possible valid value.",
			   *ParsedSeed, *InHexSeed, SeedLength);
		return 0;
	}
	// Safety Pad
	if (SeedLength % 2 != 0)
	{
		ParsedSeed.InsertAt(0, '0');
		N_LOG_VERBOSE("The seed (%s) was padded at the start; making it a valid hexadecimal seed.", *ParsedSeed);
	}

	// Generate our actual unit value, with shifting
	uint64 NewSeed = 0;
	for (int i = 0; i < ParsedSeed.Len(); i += 2)
	{
		// Get our two hexadecimal characters in the series
		const TCHAR CachedLeftCharacter = ParsedSeed[i];
		const TCHAR CachedRightCharacter = ParsedSeed[i + 1];

		// Double-check them (why?)
		if (TChar<TCHAR>::IsHexDigit(CachedLeftCharacter) && TChar<TCHAR>::IsHexDigit(CachedRightCharacter))
		{
			NewSeed = (NewSeed * 256) + (HexToInteger(CachedRightCharacter) + (HexToInteger(CachedLeftCharacter) * 16));
		}
	}

	N_LOG_VERY_VERBOSE("Generated seed (%llu) from %s.", NewSeed, *ParsedSeed);
	return NewSeed;
}

FString FNSeedGenerator::HexFromSeed(const uint64 Seed)
{
	// Let unreal handle the conversion of the uint64 to a string, it's not pretty,
	// but it alleviates all the compiler issues (%llX, %02X, %I64X) on different
	// platforms. We have to do some small operations thought to clean it up.
	const TBigInt<64, false> WorkingSeed = Seed;
	FString NewSeed = WorkingSeed.ToString();
	NewSeed.RemoveFromStart(TEXT("0x"));
	while (NewSeed.StartsWith(TEXT("0")))
	{
		NewSeed.RemoveFromStart(TEXT("0"));
	}
	NewSeed.ToUpperInline();

	// Make sure that we're right on the divisibility
	if (NewSeed.Len() % 2 != 0)
	{
		NewSeed.InsertAt(0, '0');
	}

	// Loop and add markers (:)
	FString FancySeed = TEXT("");
	for (int i = 0; i < NewSeed.Len(); i += 2)
	{
		FancySeed.AppendChar(NewSeed[i]);
		FancySeed.AppendChar(NewSeed[i + 1]);
		if (i < (NewSeed.Len() - 2))
		{
			FancySeed.AppendChar(':');
		}
	}

	return FancySeed;
}