// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_SETTINGS(Type) \
public: \
	static const Type* Get() \
	{ \
		return GetDefault<Type>(); \
	}