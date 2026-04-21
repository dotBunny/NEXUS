// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Implements a Get() static accessor that returns the UEditorSubsystem-derived Type via GEditor.
 * @param Type The editor subsystem class.
 */
#define N_EDITOR_SUBSYSTEM(Type) \
	public: \
		FORCEINLINE static Type* Get() { \
			return GEditor->GetEditorSubsystem<Type>(); \
		}

/**
 * Tickable-subsystem variant of N_EDITOR_SUBSYSTEM. Functionally identical; retained so tickable
 * subsystems are grep-distinguishable from passive ones.
 * @param Type The editor subsystem class.
 */
#define N_EDITOR_TICKABLE_SUBSYSTEM(Type) \
	public: \
		FORCEINLINE static Type* Get() { \
			return GEditor->GetEditorSubsystem<Type>(); \
		}