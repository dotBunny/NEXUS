// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

class FPlacementModeID;
class FNProcGenEditorUndo;

/**
 * NEXUS: Procedural Generation Editor Module
 */
class FNProcGenEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNProcGenEditorModule, "NexusProcGenEditor")

private:
	/** Custom undo handler that intercepts ProcGen-related transactions. */
	TSharedPtr<FNProcGenEditorUndo> UndoHandler;

	/** Handles of placement-mode entries this module registered (used to unregister on shutdown). */
	TArray<TOptional<FPlacementModeID>> PlacementActors;

	/** Engine-init callback: completes registration once the editor is fully up. */
	void OnPostEngineInit();
};