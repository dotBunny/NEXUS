// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Logging/LogMacros.h"


#define N_VALIDATE(LogCategory, Expr) \
	if (!IsValid(Expr)) \
	{ \
		UE_LOG(LogCategory, Warning, TEXT("%hs: '%hs' was null."), __FUNCTION__, #Expr); \
	}

/**
 * Guard a Blueprint-callable entry point against a null pointer/class input. If Expr fails IsValid,
 * a warning is logged via LogCategory and the function returns ReturnValue.
 *
 * @param LogCategory The log category to emit the warning on (e.g. LogNexusCore).
 * @param Expr        The pointer/class/TSubclassOf to validate.
 * @param ReturnValue The value to return when Expr is invalid.
 */
#define N_VALIDATE_RETURN(LogCategory, Expr, ReturnValue) \
	if (!IsValid(Expr)) \
	{ \
		UE_LOG(LogCategory, Warning, TEXT("%hs: '%hs' was null."), __FUNCTION__, #Expr); \
		return ReturnValue; \
	}

/**
 * Void-returning variant of N_VALIDATE_RETURN.
 *
 * @param LogCategory The log category to emit the warning on (e.g. LogNexusCore).
 * @param Expr        The pointer/class/TSubclassOf to validate.
 */
#define N_VALIDATE_RETURN_VOID(LogCategory, Expr) \
	if (!IsValid(Expr)) \
	{ \
		UE_LOG(LogCategory, Warning, TEXT("%hs: '%hs' was null."), __FUNCTION__, #Expr); \
		return; \
	}

/**
 * Skip the current loop iteration when Expr is invalid, after logging a warning. Intended for
 * Blueprint-callable entry points that accept an array of pointers/classes — Blueprint authors
 * can wire array literals containing null entries, and per-element validation lets the function
 * keep processing the valid entries.
 *
 * @param LogCategory The log category to emit the warning on.
 * @param Expr        The current iteration's pointer/class to validate.
 */
#define N_VALIDATE_CONTINUE(LogCategory, Expr) \
	if (!IsValid(Expr)) \
	{ \
		UE_LOG(LogCategory, Warning, TEXT("%hs: '%hs' contained a null entry; skipping."), __FUNCTION__, #Expr); \
		continue; \
	}