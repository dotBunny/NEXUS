// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FNOrganComponentCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply OnGenerateClicked(const TArray<TWeakObjectPtr<UObject>> Objects);
	FReply OnCancelClicked(TArray<TWeakObjectPtr<UObject>> Object);
	FReply OnClearClicked(TArray<TWeakObjectPtr<UObject>> Object);
	
	EVisibility GenerateButtonVisible() const;
	EVisibility CancelButtonVisible() const;
	EVisibility ClearButtonVisible() const;
};