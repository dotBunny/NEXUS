// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/** Evaluates to the TEXT literal "true" or "false" based on the truthiness of V. */
#define N_TRUE_FALSE(V)  (V ? TEXT("true") : TEXT("false"))

/** Stringifies V with macro expansion. Use this variant when V is itself a macro. */
#define N_STRINGIFY(V) N_VALUE(V)

/** Raw stringification helper — does not expand V before stringifying it. */
#define N_VALUE(V) #V

/** Token-pasting helper: concatenates X and Y into a single identifier. */
#define N_CONCAT(X, Y) X ## Y

/** Joins two string literals with a single space between them. */
#define N_COMBINE_STRINGS(X, Y) X Y
