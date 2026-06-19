// Copyright dotBunny Inc. All Rights Reserved.

#include "Customizations/NCellProxyCustomization.h"
#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Misc/AssertionMacros.h"
#include "DetailWidgetRow.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellProxy.h"

TSharedRef<IDetailCustomization> FNCellProxyCustomization::MakeInstance()
{
	return MakeShared<FNCellProxyCustomization>();
}

void FNCellProxyCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create or get the category
	IDetailCategoryBuilder& NexusCategory = DetailBuilder.EditCategory(TEXT("Cell Proxy"),
		FText::FromString("Cell Proxy"), ECategoryPriority::Important);

	// Figure out if we are editing multiple objects, in this case we don't want a special inspector if 
	// more then one of the components is selected.
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	TArray<TWeakObjectPtr<ANCellProxy>> Proxies;
	for (int32 i = 0; i < Objects.Num(); i++)
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

	// Nothing valid to act on, so don't add the Actions row with no-op buttons.
	if (Proxies.Num() == 0)
	{
		return;
	}

	auto OnCreate = [Proxies]
	{
		for (const TWeakObjectPtr<ANCellProxy>& CellProxy : Proxies)
		{
			if (ANCellProxy* Proxy = CellProxy.Get())
			{
				Proxy->CreateLevelInstance();
			}
		}
		return FReply::Handled();
	};

	auto OnLoad = [Proxies]
	{
		for (const TWeakObjectPtr<ANCellProxy>& CellProxy : Proxies)
		{
			if (ANCellProxy* Proxy = CellProxy.Get())
			{
				Proxy->LoadLevelInstance();
			}
		}
		return FReply::Handled();
	};

	auto OnUnload = [Proxies]
	{
		for (const TWeakObjectPtr<ANCellProxy>& CellProxy : Proxies)
		{
			if (ANCellProxy* Proxy = CellProxy.Get())
			{
				Proxy->UnloadLevelInstance();
			}
		}
		return FReply::Handled();
	};

	auto OnDestroy = [Proxies]
	{
		for (const TWeakObjectPtr<ANCellProxy>& CellProxy : Proxies)
		{
			if (ANCellProxy* Proxy = CellProxy.Get())
			{
				Proxy->DestroyLevelInstance(true, true);
			}
		}
		return FReply::Handled();
	};
	
	NexusCategory.AddCustomRow(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyActionsRow", "ActionsRow"))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyActions", "Actions"))
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
							.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyCreate", "Create Instance"))
							.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyCreateTooltip", "Create the level instance in the current level."))
							.OnClicked_Lambda(OnCreate)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.FillWidth(0.5)
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyDestroy", "Destroy Instance"))
							.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyDestroyTooltip", "Destroy the level instance in the world."))
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
							.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyLoad", "Load Level"))
							.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyLoadTooltip", "Load the level instance into the world."))
							.OnClicked_Lambda(OnLoad)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.FillWidth(0.5)
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Text(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyUnload", "Unload Level"))
							.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "ProxyUnloadTooltip", "Unload the level instance in the world."))
							.OnClicked_Lambda(OnUnload)
						//.IsEnabled_Lambda()
					]
				]
			
		];
}