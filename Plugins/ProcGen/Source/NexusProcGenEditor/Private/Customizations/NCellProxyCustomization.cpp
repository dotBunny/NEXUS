// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellProxyCustomization.h"
#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Misc/AssertionMacros.h"
#include "DetailWidgetRow.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellProxy.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

TSharedRef<IDetailCustomization> FNCellProxyCustomization::MakeInstance()
{
	return MakeShareable(new FNCellProxyCustomization());
}

void FNCellProxyCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create or get the category
	IDetailCategoryBuilder& NexusCategory = DetailBuilder.EditCategory(TEXT("NCellProxy Actions"),
		FText::GetEmpty(), ECategoryPriority::Important);

	// Figure out if we are editing multiple objects, in this case we don't want a special inspector if 
	// more then one of the components is selected.
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	TArray<TWeakObjectPtr<ANCellProxy>> Proxies;
	for (int i = 0; i < Objects.Num(); i++)
	{
		if (Objects[i].IsValid())
		{
			TWeakObjectPtr<ANCellProxy> Proxy = Cast<ANCellProxy>(Objects[i].Get());
			if (Proxy.IsValid())
			{
				Proxies.Add(Proxy);
			}
		}
	}
	
	auto OnCreate = [Proxies]
	{
		for (auto & CellProxy : Proxies)
		{
			CellProxy->CreateLevelInstance();
		}
		return FReply::Handled();
	};

	auto OnLoad = [Proxies]
	{
		for (auto & CellProxy : Proxies)
		{
			CellProxy->LoadLevelInstance();
		}
		return FReply::Handled();
	};

	auto OnUnload = [Proxies]
	{
		for (auto & CellProxy : Proxies)
		{
			CellProxy->UnloadLevelInstance();
		}
		return FReply::Handled();
	};

	auto OnDestroy = [Proxies]
	{
		for (auto & CellProxy : Proxies)
		{
			CellProxy->DestroyLevelInstance();
		}
		return FReply::Handled();
	};
	
	NexusCategory.AddCustomRow(LOCTEXT("ProxyActionsRow", "ActionsRow"))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("ProxyActions", "Actions"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent().MinDesiredWidth(500)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0,5,0,5)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.FillWidth(0.5)
					.Padding(0,0, 5,0)
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Text(LOCTEXT("ProxyCreate", "Create Instance"))
							.ToolTipText(LOCTEXT("ProxyCreateTooltip", "Create the level instance in the current level."))
							.OnClicked_Lambda(OnCreate)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.FillWidth(0.5)
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Text(LOCTEXT("ProxyDestroy", "Destroy Instance"))
							.ToolTipText(LOCTEXT("ProxyDestroyTooltip", "Destroy the level instance in the world."))
							.OnClicked_Lambda(OnDestroy)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0,0,0,5)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.FillWidth(0.5)
					.Padding(0,0, 5,0)
					[
						SNew(SButton)
							.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton"))
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Text(LOCTEXT("ProxyLoad", "Load Level"))
							.ToolTipText(LOCTEXT("ProxyLoadTooltip", "Load the level instance into the world."))
							.OnClicked_Lambda(OnLoad)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.FillWidth(0.5)
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Text(LOCTEXT("ProxyUnload", "Unload Level"))
							.ToolTipText(LOCTEXT("ProxyUnloadTooltip", "Unload the level instance in the world."))
							.OnClicked_Lambda(OnUnload)
						//.IsEnabled_Lambda()
					]
				]
			
		];
}

#undef LOCTEXT_NAMESPACE