// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Math/NSeedGenerator.h"

#include "NCoreMinimal.h"
#include "NRandom.h"
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
	return static_cast<uint64>(FNRandom::NonDeterministic.GetUnsignedInt()) << 32 | static_cast<uint64>(FNRandom::NonDeterministic.GetUnsignedInt());
}

FString FNSeedGenerator::RandomFriendlySeed()
{
	FString ReturnSeed;
	
	TArray<uint8> Digits;
	Digits.Reserve(23); // Max possible
	uint64 TempSeed = RandomSeed();;
	do
	{
		Digits.Insert(TempSeed % 10, 0);  // Insert at front to maintain order
		TempSeed /= 10;
	} while (TempSeed > 0);
	
	// Front pad digits
	while (Digits.Num() < 20)
	{
		Digits.Insert(0, 0); 
	}
	
	for (uint32 i = 0; i < 20; i++)
	{
		ReturnSeed.AppendChar(97 + Digits[i]);
		if (i == 4 || i == 9 || i == 14)
		{
			ReturnSeed.AppendChar('-');
		}
	}
	return MoveTemp(ReturnSeed);
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

uint64 FNSeedGenerator::SeedFromString(const FString& InSeed)
{
	const uint64 Seed = FNHashUtils::dbj2(InSeed);
	return Seed;
}

uint64 FNSeedGenerator::SeedFromFriendlySeed(const FString& InSeed)
{
	// Get rid of the dashes
	FString TempSeed = InSeed;
	TempSeed.ReplaceInline(TEXT("-"), TEXT(""));
	
	uint64 Seed = 0;
	
	// We're going to go in reverse as we use it as a multiplier
	uint8 Multiplier = 0;
	for (int32 i = TempSeed.Len() - 1; i >= 0; i--)
	{
		const uint8 Value = static_cast<uint8>(TempSeed[i] - 97);
		
		uint64 FactorialMultiplier = 1;
		for (int f = 0; f < Multiplier; f++)
		{
			FactorialMultiplier *= 10;
		}
		Multiplier++;
		
		Seed += (Value * FactorialMultiplier);
		
		
	}
	return Seed;
}

uint64 FNSeedGenerator::SeedFromHex(const FString& InHexSeed)
{
	// Sanitize the string
	FString ParsedSeed = SanitizeHexSeed(InHexSeed);
	const int SeedLength = ParsedSeed.Len();

	if (SeedLength == 0 || SeedLength == 1)
	{
		UE_LOG(LogNexusCore, Warning, TEXT("The parsed(%s) seed(%s) length(%i) was below any possible valid value; returning 0."), *ParsedSeed, *InHexSeed, SeedLength);
		return 0;
	}
	// Safety Pad
	if (SeedLength % 2 != 0)
	{
		ParsedSeed.InsertAt(0, '0');
		UE_LOG(LogNexusCore, Verbose, TEXT("The seed(%s) has been padded at the start; making it a valid hexadecimal seed."), *ParsedSeed);
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