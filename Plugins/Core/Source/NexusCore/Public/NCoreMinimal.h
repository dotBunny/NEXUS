// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

NEXUSCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Log, All);

#define N_LOG_VERY_VERBOSE(Format, ...) \
	UE_LOG(LogNexus, VeryVerbose, TEXT(Format),  ##__VA_ARGS__)
#define N_LOG_VERBOSE(Format, ...) \
	UE_LOG(LogNexus, Verbose, TEXT(Format),  ##__VA_ARGS__)
#define N_LOG(Format, ...) \
	UE_LOG(LogNexus, Log, TEXT(Format),  ##__VA_ARGS__)
#define N_LOG_WARNING(Format, ...) \
	UE_LOG(LogNexus, Warning, TEXT(Format),  ##__VA_ARGS__)
#define N_LOG_ERROR(Format, ...) \
	UE_LOG(LogNexus, Error, TEXT(Format),  ##__VA_ARGS__)
#define N_LOG_FATAL(Format, ...) \
	UE_LOG(LogNexus, Fatal, TEXT(Format),  ##__VA_ARGS__)

#define N_CLOG(Condition, Verbosity, Format, ...) \
	UE_CLOG(Condition, LogNexus, Verbosity, Format, ##__VA_ARGS__)

#define N_VLOG(Condition, Verbosity, Format, ...) \
	UE_VLOG(LogNexus, Verbosity, Format, ##__VA_ARGS__)

#define N_VERSION_NUMBER	5

#define N_VERSION_MAJOR		0
#define N_VERSION_MINOR		2
#define N_VERSION_PATCH		2