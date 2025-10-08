// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganContext.h"

#include "NCoreMinimal.h"
#include "Organ/NOrganComponentContext.h"

void FNOrganContext::Reset()
{
	Components.Empty();
}

void FNOrganContext::OutputLockedContext()
{
	FStringBuilderBase Builder = FStringBuilderBase();
	Builder.Append(TEXT("\n[FNOrganContext]\n"));
	Builder.Appendf(TEXT("\tComponents (%i)\n"), Components.Num());
	for (auto Pair : Components)
	{
		// Source of generation graph
		Builder.Appendf(TEXT("\t\tSource: %s\n"), *Pair.Value.SourceComponent->GetDebugLabel());
		for (auto Pair2 : Pair.Value.OtherComponents)
		{
			Builder.Appendf(TEXT("\t\t\tOtherComponent: %s\n"), *Pair2->GetDebugLabel());
		}
	}

	N_LOG(Log, TEXT("%s"), Builder.ToString());
}

