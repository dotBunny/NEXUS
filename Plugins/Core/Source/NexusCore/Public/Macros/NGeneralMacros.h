// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_TRUE_FALSE(V)  (V ? TEXT("true") : TEXT("false"))
#define N_STRINGIFY(V) N_VALUE(V)
#define N_VALUE(V) #V
#define N_CONCAT(X, Y) X ## Y
#define N_COMBINE_STRINGS(X, Y) X Y
