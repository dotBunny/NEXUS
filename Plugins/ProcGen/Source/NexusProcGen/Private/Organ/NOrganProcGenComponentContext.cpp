// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Organ/NOrganProcGenComponentContext.h"

FNOrganProcGenComponentContext::FNOrganProcGenComponentContext(UNProcGenComponent* InSourceComponent)
{
	SourceComponent = InSourceComponent;
}

void FNOrganProcGenComponentContext::Reset()
{
	OtherComponents.Empty();
}
