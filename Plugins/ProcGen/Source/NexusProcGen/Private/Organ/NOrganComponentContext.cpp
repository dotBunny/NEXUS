// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganComponentContext.h"

FNOrganComponentContext::FNOrganComponentContext(UNOrganComponent* InSourceComponent)
{
	SourceComponent = InSourceComponent;
}

void FNOrganComponentContext::Reset()
{
	IntersectComponents.Empty();
}
