// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

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
		static TSharedRef<FSlateStyleSet > Create(); \
		static TSharedPtr<FSlateStyleSet> StyleInstance; \
		static FString PluginDirectory;

#define N_IMPLEMENT_EDITOR_STYLE(Type) \
	TSharedPtr<FSlateStyleSet> Type::StyleInstance = nullptr; \
	FString Type::PluginDirectory = TEXT("");

#define N_IMPLEMENT_EDITOR_STYLE_CREATE \
	TSharedRef<FSlateStyleSet> StyleRef = MakeShareable(new FSlateStyleSet(GetStyleSetName())); \
	FSlateStyleSet& Style = StyleRef.Get(); \
	const FVector2D Icon128x128(128.0f, 128.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon32x32(32.0f, 32.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);

#define N_MODULE_IMAGE_BRUSH_PNG( BaseDirectory, RelativePath, ... ) \
	FSlateImageBrush( BaseDirectory / "Resources" / RelativePath + TEXT(".png"), __VA_ARGS__ )

#define N_MODULE_IMAGE_BRUSH_SVG( BaseDirectory, RelativePath, ... ) \
	FSlateVectorImageBrush(  BaseDirectory / "Resources"  / RelativePath + TEXT(".svg"), __VA_ARGS__ )
