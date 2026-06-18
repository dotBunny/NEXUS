// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "IDetailCustomization.h"

/**
 * Detail-panel customization for UNWorldAssemblyEditorUserSettings. Restores the per-property
 * "reset to default" arrow for the config-backed color properties.
 *
 * Unreal disables the built-in reset arrow for any UPROPERTY flagged `config`
 * (FPropertyHandleBase::CanResetToDefault gates on CPF_Config), so without this the color settings
 * can never be reverted to their C++ defaults from the Editor Preferences panel. We install an
 * FResetToDefaultOverride per color row, which the reset widget honors in place of the suppressed
 * default path.
 */
class FNWorldAssemblyEditorUserSettingsCustomization final : public IDetailCustomization
{
public:
	/** Factory entry point registered with the property editor module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//End IDetailCustomization

private:
	/**
	 * Installs an FResetToDefaultOverride on a config-backed FLinearColor property so it shows and
	 * handles the reset arrow despite carrying the config flag.
	 * @param DetailBuilder The layout builder for the panel being customized.
	 * @param PropertyName The FLinearColor config property to override.
	 * @param DefaultColor The C++ default value to compare against and revert to.
	 */
	static void OverrideColorReset(IDetailLayoutBuilder& DetailBuilder, FName PropertyName, const FLinearColor& DefaultColor);
};
