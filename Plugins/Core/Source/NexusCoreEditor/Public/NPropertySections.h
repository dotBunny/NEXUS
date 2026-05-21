// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class NEXUSCOREEDITOR_API FNPropertySections
{
public:
	static void Register();
	
	static void AddCategory(FName Category);
	static void AddActorCategory(FName Category);
	static void AddActorComponentCategory(FName Category);
	static void AddSceneComponentCategory(FName Category);
	static void AddObjectCategory(FName Category);
	
private:
	static TSharedPtr<FPropertySection> ActorPropertySection;
	static TSharedPtr<FPropertySection> ActorComponentPropertySection;
	static TSharedPtr<FPropertySection> SceneComponentPropertySection;
	static TSharedPtr<FPropertySection> ObjectPropertySection;
	
	static const FText DisplayName;
	static const FName Identifier;
	static const FName BaseCategory;
	
	static bool bHasRegistered;
	
	static TArray<FName> PendingActors;
	static TArray<FName> PendingActorComponents;
	static TArray<FName> PendingSceneComponents;
	static TArray<FName> PendingObjectComponents;
};