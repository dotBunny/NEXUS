// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Wires a UDeveloperSettings-derived runtime settings class into the Project Settings UI with cached singleton access.
 *
 * Injects a cached-singleton Get()/GetMutable() pair plus the four virtual overrides Unreal looks at when
 * placing a settings class in the Project Settings tree (container, category, section title, section
 * description). The cached pointers are populated lazily from the engine's CDO machinery and held as
 * function-scope statics so the first access pays the cost and subsequent reads are cheap.
 *
 * Container and category come from FNSettingsUtils so every NEXUS runtime settings class lands under
 * the same shared section. Title and Description are baked into FText literals at compile time, which
 * is why they must be string literals rather than variables.
 *
 * Section title and description are only emitted when WITH_EDITOR is defined — in non-editor builds the
 * Project Settings UI does not exist, so those overrides are omitted to avoid pulling FText machinery
 * into a runtime build.
 *
 * Place in the public section of a settings class's body.
 *
 * @param Type        The settings class itself (used to infer template args for GetDefault/GetMutableDefault).
 * @param Title       String literal shown as the settings section title in the editor's Project Settings tree.
 * @param Description String literal shown as the section description / tooltip in the editor's Project Settings tree.
 */
#if WITH_EDITOR
#define N_IMPLEMENT_SETTINGS(Type, Title, Description) \
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
		} \
		virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); } \
		virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  } \
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
#define N_IMPLEMENT_SETTINGS(Type, Title, Description) \
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
		} \
		virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); } \
		virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }
#endif // WITH_EDITOR

/**
 * Wires a UDeveloperSettings-derived runtime settings class into the Project Settings UI with cached singleton access.
 *
 * Injects a cached-singleton Get()/GetMutable() pair plus the four virtual overrides Unreal looks at when
 * placing a settings class in the Project Settings tree (container, category, section title, section
 * description). The cached pointers are populated lazily from the engine's CDO machinery and held as
 * function-scope statics so the first access pays the cost and subsequent reads are cheap.
 *
 * Container, Category, Title and Description are baked into FText literals at compile time, which
 * is why they must be string literals rather than variables.
 *
 * Section title and description are only emitted when WITH_EDITOR is defined — in non-editor builds the
 * Project Settings UI does not exist, so those overrides are omitted to avoid pulling FText machinery
 * into a runtime build.
 *
 * Place in the public section of a settings class's body.
 *
 * @param Type        The settings class itself (used to infer template args for GetDefault/GetMutableDefault).
 * @param Container   String litteral settings container name, most likely this is going to be "Project".
 * @param Category    String literal used as the category in the editor's Project Settings tree. 
 * @param Title       String literal shown as the settings section title in the editor's Project Settings tree.
 * @param Description String literal shown as the section description / tooltip in the editor's Project Settings tree.
 */
#if WITH_EDITOR
#define N_IMPLEMENT_CUSTOM_SETTINGS(Type, Container, Category, Title, Description) \
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
		} \
		virtual FName GetContainerName() const override { return Container; } \
		virtual FName GetCategoryName() const override {  return Category;  } \
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
#define N_IMPLEMENT_CUSTOM_SETTINGS(Type, Container, Category, Title, Description) \
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
		} \
		virtual FName GetContainerName() const override { return Container; } \
		virtual FName GetCategoryName() const override {  return Category;  }
#endif // WITH_EDITOR