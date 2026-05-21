#include "NPropertySections.h"

const FText FNPropertySections::DisplayName = FText::FromString(TEXT("NEXUS"));
const FName FNPropertySections::Identifier = FName(TEXT("NEXUS"));
const FName FNPropertySections::BaseCategory = FName(TEXT("NEXUS"));

TSharedPtr<FPropertySection> FNPropertySections::ActorPropertySection;
TSharedPtr<FPropertySection> FNPropertySections::ActorComponentPropertySection;
TSharedPtr<FPropertySection> FNPropertySections::SceneComponentPropertySection;
TSharedPtr<FPropertySection> FNPropertySections::ObjectPropertySection;

TArray<FName> FNPropertySections::PendingActors;
TArray<FName> FNPropertySections::PendingActorComponents;
TArray<FName> FNPropertySections::PendingSceneComponents;
TArray<FName> FNPropertySections::PendingObjectComponents;

bool FNPropertySections::bHasRegistered = false;

void FNPropertySections::Register()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	ActorPropertySection = PropertyModule.FindOrCreateSection(FName("Actor"), Identifier, DisplayName);
	ActorPropertySection->AddCategory(BaseCategory);
	if (!PendingActors.IsEmpty())
	{
		for (FName Entry : PendingActors)
		{
			if (!ActorPropertySection->HasAddedCategory(Entry))
			{
				ActorPropertySection->AddCategory(Entry);
			}
		}
		PendingActors.Empty();
	}
	
	ActorComponentPropertySection = PropertyModule.FindOrCreateSection(FName("ActorComponent"), Identifier, DisplayName);
	ActorComponentPropertySection->AddCategory(BaseCategory);
	if (!PendingActorComponents.IsEmpty())
	{
		for (FName Entry : PendingActorComponents)
		{
			if (!ActorComponentPropertySection->HasAddedCategory(Entry))
			{
				ActorComponentPropertySection->AddCategory(Entry);
			}
		}
		PendingActorComponents.Empty();
	}
	
	SceneComponentPropertySection = PropertyModule.FindOrCreateSection(FName("SceneComponent"), Identifier, DisplayName);
	SceneComponentPropertySection->AddCategory(BaseCategory);
	if (!PendingSceneComponents.IsEmpty())
	{
		for (FName Entry : PendingSceneComponents)
		{
			if (!SceneComponentPropertySection->HasAddedCategory(Entry))
			{
				SceneComponentPropertySection->AddCategory(Entry);
			}
		}
		PendingSceneComponents.Empty();
	}
	
	ObjectPropertySection = PropertyModule.FindOrCreateSection(FName("Object"), Identifier, DisplayName);
	ObjectPropertySection->AddCategory(BaseCategory);
	if (!PendingObjectComponents.IsEmpty())
	{
		for (FName Entry : PendingObjectComponents)
		{
			if (!ObjectPropertySection->HasAddedCategory(Entry))
			{
				ObjectPropertySection->AddCategory(Entry);
			}
		}
		PendingObjectComponents.Empty();
	}
	
	bHasRegistered = true;
}

void FNPropertySections::AddCategory(const FName Category)
{
	AddActorCategory(Category);
	AddActorComponentCategory(Category);
	AddSceneComponentCategory(Category);
	AddObjectCategory(Category);
}

void FNPropertySections::AddActorCategory(const FName Category)
{
	if (!bHasRegistered)
	{
		PendingActors.AddUnique(Category);
		return;
	}
	
	if (!ActorPropertySection->HasAddedCategory(Category))
	{
		ActorPropertySection->AddCategory(Category);
	}
}

void FNPropertySections::AddActorComponentCategory(const FName Category)
{
	if (!bHasRegistered)
	{
		PendingActorComponents.AddUnique(Category);
		return;
	}
	
	if (!ActorComponentPropertySection->HasAddedCategory(Category))
	{
		ActorComponentPropertySection->AddCategory(Category);
	}
}

void FNPropertySections::AddSceneComponentCategory(const FName Category)
{
	if (!bHasRegistered)
	{
		PendingSceneComponents.AddUnique(Category);
		return;
	}
	
	if (!SceneComponentPropertySection->HasAddedCategory(Category))
	{
		SceneComponentPropertySection->AddCategory(Category);
	}
}

void FNPropertySections::AddObjectCategory(FName Category)
{
	if (!bHasRegistered)
	{
		PendingObjectComponents.AddUnique(Category);
		return;
	}
	
	if (!ObjectPropertySection->HasAddedCategory(Category))
	{
		ObjectPropertySection->AddCategory(Category);
	}
}
