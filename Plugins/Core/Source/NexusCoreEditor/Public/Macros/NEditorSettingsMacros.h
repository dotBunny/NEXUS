// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Implements the canonical Get() / GetMutable() static accessor pair for a UDeveloperSettings-derived editor settings class.
 *
 * Get() returns the immutable CDO for read access; GetMutable() lazily caches a mutable pointer
 * for sites that need to write configuration values.
 *
 * @param Type The UDeveloperSettings subclass that receives the accessors.
 */
#define N_IMPLEMENT_EDITOR_SETTINGS(Type) \
	public: \
		static const Type* Get() \
		{ \
			return GetDefault<Type>(); \
		} \
		static Type* GetMutable() { \
			static Type* MutableInstance; \
			if (MutableInstance == nullptr) \
			{ \
				MutableInstance = GetMutableDefault<Type>(); \
			} \
			return MutableInstance; \
		}
