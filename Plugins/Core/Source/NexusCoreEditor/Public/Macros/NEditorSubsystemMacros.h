// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_EDITOR_SUBSYSTEM(Type) \
	public: \
		FORCEINLINE static Type* Get() { \
			return GEditor->GetEditorSubsystem<Type>(); \
		}
#define N_EDITOR_TICKABLE_SUBSYSTEM(Type) \
	public: \
		FORCEINLINE static Type* Get() { \
			return GEditor->GetEditorSubsystem<Type>(); \
		}