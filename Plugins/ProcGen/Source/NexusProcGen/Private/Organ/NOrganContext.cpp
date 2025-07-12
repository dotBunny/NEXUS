// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganContext.h"

#include "NCoreMinimal.h"
#include "Organ/NOrganProcGenComponentContext.h"

void FNOrganContext::Reset()
{
	Components.Empty();
}

void FNOrganContext::OutputLockedContext()
{
	FStringBuilderBase Builder = FStringBuilderBase();

	Builder.Append(TEXT("\nFNOrganContext\n"));

	Builder.Append(TEXT("\tComponents (%s)\n"));
	for (auto Pair : Components)
	{
		Builder.Appendf(TEXT("\t\tSource: %s\n"), *Pair.Value.SourceComponent->GetDebugLabel());
		for (auto Pair2 : Pair.Value.OtherComponents)
		{
			Builder.Appendf(TEXT("\t\t\tOtherComponent: %s\n"), *Pair2->GetDebugLabel());
		}
	}

	N_LOG(Log, TEXT("%s"), Builder.ToString());
}

