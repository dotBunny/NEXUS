// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

NEXUSCOREEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusEditor, Log, All);

// Logging
#define NE_LOG_VERY_VERBOSE(Format, ...) \
	UE_LOG(LogNexusEditor, VeryVerbose, TEXT(Format),  ##__VA_ARGS__)
#define NE_LOG_VERBOSE(Format, ...) \
	UE_LOG(LogNexusEditor, Verbose, TEXT(Format),  ##__VA_ARGS__)
#define NE_LOG(Format, ...) \
	UE_LOG(LogNexusEditor, Log, TEXT(Format),  ##__VA_ARGS__)
#define NE_LOG_WARNING(Format, ...) \
	UE_LOG(LogNexusEditor, Warning, TEXT(Format),  ##__VA_ARGS__)
#define NE_LOG_ERROR(Format, ...) \
	UE_LOG(LogNexusEditor, Error, TEXT(Format),  ##__VA_ARGS__)
#define NE_LOG_FATAL(Format, ...) \
	UE_LOG(LogNexusEditor, Fatal, TEXT(Format),  ##__VA_ARGS__)

// Conditional Logging
#define NE_CLOG_VERY_VERBOSE(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define NE_CLOG_VERBOSE(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, Verbose, TEXT(Format), ##__VA_ARGS__)
#define NE_CLOG(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, Log, TEXT(Format), ##__VA_ARGS__)
#define NE_CLOG_WARNING(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, Warning, TEXT(Format), ##__VA_ARGS__)
#define NE_CLOG_ERROR(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, Error, TEXT(Format), ##__VA_ARGS__)
#define NE_CLOG_FATAL(Condition, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, Fatal, TEXT(Format), ##__VA_ARGS__)

// Visual Logger
#define NE_VLOG_VERY_VERBOSE(Format, ...) \
	UE_VLOG(LogNexusEditor, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define NE_VLOG_VERBOSE(Format, ...) \
	UE_VLOG(LogNexusEditor, Verbose, TEXT(Format), ##__VA_ARGS__)
#define NE_VLOG(Format, ...) \
	UE_VLOG(LogNexusEditor, Log, TEXT(Format), ##__VA_ARGS__)
#define NE_VLOG_WARNING(Format, ...) \
	UE_VLOG(LogNexusEditor, Warning, TEXT(Format), ##__VA_ARGS__)
#define NE_VLOG_ERROR(Format, ...) \
	UE_VLOG(LogNexusEditor, Error, TEXT(Format), ##__VA_ARGS__)
#define NE_VLOG_FATAL(Format, ...) \
	UE_VLOG(LogNexusEditor, Fatal, TEXT(Format), ##__VA_ARGS__)

// Conditional Visual Logger
#define NE_CVLOG_VERY_VERBOSE(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexusEditor, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define NE_CVLOG_VERBOSE(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexusEditor, Verbose, TEXT(Format), ##__VA_ARGS__)
#define NE_CVLOG(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexusEditor, Log, TEXT(Format), ##__VA_ARGS__)
#define NE_CVLOG_WARNING(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexusEditor, Warning, TEXT(Format), ##__VA_ARGS__)
#define NE_CVLOG_ERROR(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexusEditor, Error, TEXT(Format), ##__VA_ARGS__)
#define NE_CVLOG_FATAL(Condition, Format, ...) \
	UE_CVLOG(Condition, LogNexusEditor, Fatal, TEXT(Format), ##__VA_ARGS__)