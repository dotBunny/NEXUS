// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Injects a cached-singleton Get()/GetMutable() pair into a UDeveloperSettings-derived type.
 *
 * The cached pointers are populated lazily from the engine's CDO machinery and held as function-scope
 * statics so the first access pays the cost and subsequent reads are cheap. Place in the public
 * section of a settings class's body.
 *
 * @param Type The settings class itself (used to infer template args for GetDefault/GetMutableDefault).
 */
#define N_IMPLEMENT_SETTINGS(Type) \
public: \
	static const Type* Get() \
	{ \
		static const Type* CachedSettings = GetDefault<Type>(); \
		return CachedSettings; \
	} \
	static Type* GetMutable() { \
		static Type* MutableInstance; \
		if (MutableInstance == nullptr) \
		{ \
			MutableInstance = GetMutableDefault<Type>(); \
		} \
		return MutableInstance; \
	}