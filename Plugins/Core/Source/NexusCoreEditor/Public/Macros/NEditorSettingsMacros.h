// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Wires a UDeveloperSettings-derived editor settings class into the Editor Preferences UI with static accessors.
 *
 * Injects the canonical Get() / GetMutable() pair plus the virtual overrides Unreal needs to place the
 * class under the shared NEXUS editor settings container / category. Get() returns the immutable CDO for
 * read access; GetMutable() lazily caches a mutable pointer for sites that need to write configuration
 * values.
 *
 * Container and category come from FNEditorDefaults so every NEXUS editor settings class lands in the
 * same section of Editor Preferences. Title and Description are baked into FText literals at compile
 * time, which is why they must be string literals rather than variables.
 *
 * In WITH_EDITOR builds, the macro also emits GetSectionText / GetSectionDescription so the entry shows
 * up properly in the editor UI. The non-editor variant exists so the same class compiles in cooked
 * runtime builds, where the section overrides are unnecessary and editor-only FText machinery is
 * stripped.
 *
 * Place in the public section of an editor settings class's body.
 *
 * @param Type        The UDeveloperSettings subclass that receives the accessors.
 * @param Title       String literal shown as the section title in Editor Preferences.
 * @param Description String literal shown as the section description / tooltip in Editor Preferences.
 */
#if WITH_EDITOR
#define N_IMPLEMENT_EDITOR_SETTINGS(Type, Title, Description) \
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
		} \
		virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); } \
		virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  } \
		virtual FText GetSectionText() const override \
		{ \
			const FText SectionText =  FText::FromString(TEXT(Title)); \
			return SectionText; \
		} \
		virtual FText GetSectionDescription() const override \
		{ \
			const FText SectionDescription = FText::FromString(TEXT(Description)); \
			return SectionDescription; \
		}
#else
#define N_IMPLEMENT_EDITOR_SETTINGS(Type, Title, Description) \
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
		} \
		virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); } \
		virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsContainerName();  }
#endif // WITH_EDITOR



/**
 * Variant of N_IMPLEMENT_EDITOR_SETTINGS for classes that should not appear anywhere in the Editor Preferences UI.
 *
 * Returns NAME_None for both GetContainerName and GetCategoryName, which causes Unreal's settings registry
 * to skip the class when building the preferences tree. The class still gets the standard Get() /
 * GetMutable() accessors, so it remains usable as a config-backed UDeveloperSettings — just invisible to
 * end users. Useful for internal / per-user state that needs to round-trip through the .ini system but
 * that we do not want exposed as a configurable surface.
 *
 * Place in the public section of the settings class's body.
 *
 * @param Type The UDeveloperSettings subclass that receives the accessors.
 */
#define N_IMPLEMENT_HIDDEN_EDITOR_SETTINGS(Type) \
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
		} \
		virtual FName GetContainerName() const override { return NAME_None; } \
		virtual FName GetCategoryName() const override {  return NAME_None;  }