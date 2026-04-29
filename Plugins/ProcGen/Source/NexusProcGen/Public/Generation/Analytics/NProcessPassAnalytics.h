// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenTaskTimer.h"

struct FNProcessPassAnalytics
{
	int Phase = 0;
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
};
