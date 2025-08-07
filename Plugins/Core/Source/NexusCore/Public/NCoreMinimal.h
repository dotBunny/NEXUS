// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

NEXUSCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Log, All);

#define N_LOG(Verbosity, Format, ...) \
	UE_LOG(LogNexus, Verbosity, Format, ##__VA_ARGS__)
#define N_CLOG(Condition, Verbosity, Format, ...) \
	UE_CLOG(Condition, LogNexus, Verbosity, Format, ##__VA_ARGS__)
#define N_VLOG(Condition, Verbosity, Format, ...) \
	UE_VLOG(LogNexus, Verbosity, Format, ##__VA_ARGS__)

#define N_VERSION_NUMBER	2

#define N_VERSION_MAJOR		0
#define N_VERSION_MINOR		1
#define N_VERSION_PATCH		1