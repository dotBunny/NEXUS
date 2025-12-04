// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

NEXUSCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Log, All);

// Logging
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

// Conditional Logging
#define N_CLOG_VERY_VERBOSE(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexus, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define N_CLOG_VERBOSE(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexus, Verbose, TEXT(Format), ##__VA_ARGS__)
#define N_CLOG(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexus, Log, TEXT(Format), ##__VA_ARGS__)
#define N_CLOG_WARNING(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexus, Warning, TEXT(Format), ##__VA_ARGS__)
#define N_CLOG_ERROR(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexus, Error, TEXT(Format), ##__VA_ARGS__)
#define N_CLOG_FATAL(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexus, Fatal, TEXT(Format), ##__VA_ARGS__)

// Visual Logger
#define N_VLOG_VERY_VERBOSE(Format, ...) \
	UE_VLOG(LogNexus, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define N_VLOG_VERBOSE(Format, ...) \
	UE_VLOG(LogNexus, Verbose, TEXT(Format), ##__VA_ARGS__)
#define N_VLOG(Format, ...) \
	UE_VLOG(LogNexus, Log, TEXT(Format), ##__VA_ARGS__)
#define N_VLOG_WARNING(Format, ...) \
	UE_VLOG(LogNexus, Warning, TEXT(Format), ##__VA_ARGS__)
#define N_VLOG_ERROR(Format, ...) \
	UE_VLOG(LogNexus, Error, TEXT(Format), ##__VA_ARGS__)
#define N_VLOG_FATAL(Format, ...) \
	UE_VLOG(LogNexus, Fatal, TEXT(Format), ##__VA_ARGS__)

// Conditional Visual Logger
// ReSharper disable IdentifierTypo
#define N_CVLOG_VERY_VERBOSE(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexus, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define N_CVLOG_VERBOSE(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexus, Verbose, TEXT(Format), ##__VA_ARGS__)
#define N_CVLOG(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexus, Log, TEXT(Format), ##__VA_ARGS__)
#define N_CVLOG_WARNING(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexus, Warning, TEXT(Format), ##__VA_ARGS__)
#define N_CVLOG_ERROR(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexus, Error, TEXT(Format), ##__VA_ARGS__)
#define N_CVLOG_FATAL(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexus, Fatal, TEXT(Format), ##__VA_ARGS__)
// ReSharper restore IdentifierTypo

#define N_VERSION_NUMBER	5

#define N_VERSION_MAJOR		0
#define N_VERSION_MINOR		2
#define N_VERSION_PATCH		2