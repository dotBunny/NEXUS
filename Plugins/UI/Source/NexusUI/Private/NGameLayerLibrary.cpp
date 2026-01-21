#include "NGameLayerLibrary.h"

#include "NWidgetUtils.h"
#include "Components/Widget.h"
#include "Slate/SGameLayerManager.h"

bool UNGameLayerLibrary::SetLayerVisibility(ULocalPlayer* LocalPlayer, const FName Name, const ESlateVisibility Visibility)
{
	if (LocalPlayer == nullptr) return false;
	
	const UWorld* World = LocalPlayer->GetWorld();
	if (World == nullptr || !World->IsGameWorld()) return false;
	
	const UGameViewportClient* ViewportClient = World->GetGameViewport();
	if (ViewportClient == nullptr) return false;
	
	const TSharedPtr<IGameLayerManager> LayerManager = ViewportClient->GetGameLayerManager();
	if (!LayerManager.IsValid()) return false;
	
	const TSharedPtr<IGameLayer> Layer = LayerManager->FindLayerForPlayer(LocalPlayer, Name);
	if (!Layer.IsValid()) return false;
	
	Layer->AsWidget()->SetVisibility(FNWidgetUtils::ToEVisibility(Visibility));
	
	return true;
}
