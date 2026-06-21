// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Customizations/NWorldAssemblyEditorUserSettingsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "NWorldAssemblyEditorUserSettings.h"

TSharedRef<IDetailCustomization> FNWorldAssemblyEditorUserSettingsCustomization::MakeInstance()
{
	return MakeShared<FNWorldAssemblyEditorUserSettingsCustomization>();
}

void FNWorldAssemblyEditorUserSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// The reset-to-default arrow is suppressed by the engine for `config` properties; re-enable it per
	// color row by overriding it explicitly. Defaults mirror the member initializers in the settings header.
	using Settings = UNWorldAssemblyEditorUserSettings;

	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteBonesValid), NEXUS::WorldAssembly::DefaultColors::BoneValid);
	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteBonesInvalid), NEXUS::WorldAssembly::DefaultColors::BoneInvalid);

	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteCellBounds), NEXUS::WorldAssembly::DefaultColors::CellBounds);
	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteCellHull), NEXUS::WorldAssembly::DefaultColors::CellHull);

	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteJunctionsUnfilled), NEXUS::WorldAssembly::DefaultColors::JunctionUnfilled);
	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteJunctionsValid), NEXUS::WorldAssembly::DefaultColors::JunctionValid);
	OverrideColorReset(DetailBuilder, GET_MEMBER_NAME_CHECKED(Settings, ColorPaletteJunctionsInvalid), NEXUS::WorldAssembly::DefaultColors::JunctionInvalid);
}

void FNWorldAssemblyEditorUserSettingsCustomization::OverrideColorReset(
	IDetailLayoutBuilder& DetailBuilder, FName PropertyName, const FLinearColor& DefaultColor)
{
	const TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(PropertyName);
	if (!PropertyHandle->IsValidHandle())
	{
		return;
	}

	const FResetToDefaultOverride Override = FResetToDefaultOverride::Create(
		FIsResetToDefaultVisible::CreateLambda([DefaultColor](TSharedPtr<IPropertyHandle> Handle)
		{
			void* ValueData = nullptr;
			if (Handle->GetValueData(ValueData) == FPropertyAccess::Success && ValueData != nullptr)
			{
				return !static_cast<const FLinearColor*>(ValueData)->Equals(DefaultColor);
			}
			return false;
		}),
		FResetToDefaultHandler::CreateLambda([DefaultColor](TSharedPtr<IPropertyHandle> Handle)
		{
			// Route through the handle so the change is transacted, written to config, and fires
			// PostEditChangeProperty (which refreshes FNWorldAssemblyEdMode's cached colors).
			Handle->SetValueFromFormattedString(DefaultColor.ToString());
		}));

	DetailBuilder.EditDefaultProperty(PropertyHandle)->OverrideResetToDefault(Override);
}
