// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

NEXUSCOREEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusEditor, Log, All);

#define NE_LOG(Verbosity, Format, ...) \
	UE_LOG(LogNexusEditor, Verbosity, Format,  ##__VA_ARGS__)
#define NE_CLOG(Condition, Verbosity, Format, ...) \
	UE_CLOG(Condition, LogNexusEditor, Verbosity, Format, ##__VA_ARGS__)
#define NE_VLOG(Condition, Verbosity, Format, ...) \
	UE_VLOG(LogNexusEditor, Verbosity, Format, ##__VA_ARGS__)