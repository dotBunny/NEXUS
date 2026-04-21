// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Implements the standard boilerplate (Initialize/Shutdown/Get/GetStyleSetName/GetStyleInstance)
 * for a NEXUS Slate style class.
 *
 * Pair with N_IMPLEMENT_EDITOR_STYLE and N_IMPLEMENT_EDITOR_STYLE_CREATE in the matching .cpp.
 *
 * @param ModuleName The plugin's module name; used to locate the plugin base directory.
 * @param StyleName The Slate style set's registered name.
 */
#define N_IMPLEMENT_EDITOR_STYLE_HEADER(ModuleName, StyleName) \
	public: \
		static void Initialize() \
		{ \
			if (!StyleInstance.IsValid()) \
			{ \
				PluginDirectory = IPluginManager::Get().FindPlugin(TEXT(ModuleName))->GetBaseDir(); \
				StyleInstance = Create(); \
				FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance); \
			} \
		} \
		static void Shutdown() \
		{ \
			if(StyleInstance == nullptr) return; \
			FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance); \
			ensure(StyleInstance.IsUnique()); \
			StyleInstance.Reset(); \
		} \
		static const ISlateStyle& Get() { return *StyleInstance; } \
		static FName GetStyleSetName() \
		{ \
			static FName StyleSetName(TEXT(StyleName)); \
			return StyleSetName; \
		} \
		static TSharedPtr<FSlateStyleSet> GetStyleInstance() { return StyleInstance; }; \
	private: \
		static TSharedRef<FSlateStyleSet> Create(); \
		static TSharedPtr<FSlateStyleSet> StyleInstance; \
		static FString PluginDirectory;

/**
 * Allocates the static storage declared by N_IMPLEMENT_EDITOR_STYLE_HEADER.
 * Place in the style class's .cpp file.
 * @param Type The style class.
 */
#define N_IMPLEMENT_EDITOR_STYLE(Type) \
	TSharedPtr<FSlateStyleSet> Type::StyleInstance = nullptr; \
	FString Type::PluginDirectory = TEXT("");

/**
 * Opens a Create() body by declaring the style set and common icon-size vectors.
 *
 * Use at the top of Type::Create() to reduce boilerplate when authoring Slate style sets.
 */
#define N_IMPLEMENT_EDITOR_STYLE_CREATE \
	TSharedRef<FSlateStyleSet> StyleRef = MakeShareable(new FSlateStyleSet(GetStyleSetName())); \
	FSlateStyleSet& Style = StyleRef.Get(); \
	const FVector2D Icon128x128(128.0f, 128.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon32x32(32.0f, 32.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);

/**
 * Builds an FSlateImageBrush from a PNG located under `<BaseDirectory>/Resources/<RelativePath>.png`.
 * @param BaseDirectory Absolute base directory (typically the plugin root).
 * @param RelativePath Path under Resources, without extension.
 */
#define N_MODULE_IMAGE_BRUSH_PNG( BaseDirectory, RelativePath, ... ) \
	FSlateImageBrush( BaseDirectory / "Resources" / RelativePath + TEXT(".png"), __VA_ARGS__ )

/**
 * Builds an FSlateVectorImageBrush from an SVG located under `<BaseDirectory>/Resources/<RelativePath>.svg`.
 * @param BaseDirectory Absolute base directory (typically the plugin root).
 * @param RelativePath Path under Resources, without extension.
 */
#define N_MODULE_IMAGE_BRUSH_SVG( BaseDirectory, RelativePath, ... ) \
	FSlateVectorImageBrush(  BaseDirectory / "Resources"  / RelativePath + TEXT(".svg"), __VA_ARGS__ )
