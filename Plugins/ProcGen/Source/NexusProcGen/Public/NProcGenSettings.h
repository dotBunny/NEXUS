// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "Types/NDirection.h"
#include "NProcGenSettings.generated.h"


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
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Voxel Size",
		meta=(ToolTip="What is the size to use when generating voxel data around the space of a NCell. This sizing will also be used to calculate some additional meta data as a unit size. It doesn't need to be too tight; just remember the smaller the size, the greater the performance hit."))
	FVector VoxelSize = FVector(100.f, 100.f, 100.f);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Socket Size",
		meta=(ToolTip="What is the unit base size for things like the NJunctions and NBones."))
	FVector2D SocketSize = FVector2D(50.f, 50.f);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName="Player Size",
		meta=(ToolTip="What is the size of the player's collider?"))
	FVector PlayerSize = FVector(72.f, 184.f, 72.f);
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ", DisplayName="Automatic Bone Direction",
	meta=(ToolTip="The direction used to calculate the automatic bone placement on the volume."))
	ENDirection OrganAutomaticBoneDirection = ENDirection::Backward;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Organ", DisplayName="Automatic Bone Direction (Offset)",
		meta=(ToolTip="Offset value applied to the direction provided by the enumeration."))
	FVector OrganAutomaticBoneDirectionOffset = FVector::ZeroVector;
	
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