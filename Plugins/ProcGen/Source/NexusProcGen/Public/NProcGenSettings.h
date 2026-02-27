// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "NProcGenSettings.generated.h"

UENUM(BlueprintType)
enum class ENBoneAutomaticAlignment : uint8
{
	Center = 0,
	Minimum = 1,
	Maximum = 2
};


UCLASS(ClassGroup = "NEXUS", DisplayName = "ProcGen Settings", Config=NexusGame, defaultconfig)
class NEXUSPROCGEN_API UNProcGenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_IMPLEMENT_SETTINGS(UNProcGenSettings);

#if WITH_EDITOR
	
	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();
		ValidateSettings();
	}
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		ValidateSettings();
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}

	virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); }
	virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }
	
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Procedural Generation"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Settings related to ProcGen."));
		return SectionDescription;
	}

#endif // WITH_EDITOR

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Generation Settings", DisplayName="Unit Size",
		meta=(ToolTip="What is the base unit sized when operating on our grid?"))
	FVector UnitSize = FVector(50.f, 50.f, 50.f);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Generation Settings", DisplayName="Player Size",
		meta=(ToolTip="What is the size of the player's collider?"))
	FVector PlayerSize = FVector(72.f, 184.f, 72.f);
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ Settings", DisplayName="Automatic Bone: Horizontal Alignment",
		meta=(ToolTip="If an Automatic UBoneComponent is being used, what horizontal alignment should be used on the target face."))
	ENBoneAutomaticAlignment  OrganAutomaticBoneHorizontalAlignment = ENBoneAutomaticAlignment::Center;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ Settings", DisplayName="Automatic Bone: Vertical Alignment",
		meta=(ToolTip="If an Automatic UBoneComponent is being used, what vertical alignment should be used on the target face."))
	ENBoneAutomaticAlignment  OrganAutomaticBoneVerticalAlignment = ENBoneAutomaticAlignment::Center;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ Settings", DisplayName="Automatic Bone: Face Direction",
		meta=(ToolTip="The direction of the face of the volume to use for placement."))
	FVector OrganAutomaticBoneDirection = FVector::ZeroVector;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ Settings", DisplayName="Automatic Bone: Unit Offset",
		meta=(ToolTip="The unit size offset from the alignment pivot of the automatic bone."))
	FVector OrganAutomaticBoneUnitOffset = FVector::ZeroVector;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "Debug", DisplayName="Proxy Material")
	TSoftObjectPtr<UMaterial> ProxyMaterial;
	
#if WITH_EDITOR
private:
	void ValidateSettings()
	{
		bool bNeedsSave = false;

		if (!ProxyMaterial)
		{
			UObject* DefaultProxyMaterial = FSoftClassPath(
				TEXT("/NexusProcGen/M_NCellProxy.M_NCellProxy"))
				.TryLoad();
			if (DefaultProxyMaterial != nullptr)
			{
				bNeedsSave = true;
				ProxyMaterial = Cast<UMaterial>(DefaultProxyMaterial);
			}
		}
		
		if (bNeedsSave)
		{
			TryUpdateDefaultConfigFile();
		}
	}
#endif // WITH_EDITOR
};