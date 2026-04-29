// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FNProcGenTaskGraphContext;
class UNProcGenOperation;

/**
 * Interface implemented by anything that owns a UNProcGenOperation (runtime subsystem, editor subsystem,
 * editor mode). The operation routes its lifecycle callbacks back to the owner so runtime vs editor
 * driven flows can specialize their behavior.
 * @remark Native-only interface; not exposed to Blueprints.
 */
class INProcGenOperationOwner
{
public:
	INProcGenOperationOwner() = default;
	virtual ~INProcGenOperationOwner() = default;

	/**
	 * Called when the operation is ready to begin executing its task graph.
	 * @param Operation The operation that is starting.
	 */
	virtual void StartOperation(UNProcGenOperation* Operation) = 0;

	/**
	 * Called after all tasks in the operation have completed.
	 * @param Operation The operation that finished.
	 * @param TaskGraphContext Context carrying the results of the completed task graph.
	 */
	virtual void OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext) = 0;

	/**
	 * Called when the operation is being destroyed so the owner can drop references and run cleanup.
	 * @param Operation The operation being destroyed.
	 */
	virtual void OnOperationDestroyed(UNProcGenOperation* Operation) = 0;

	/** @return The world the owner considers authoritative for any operation it drives. */
	virtual UWorld* GetDefaultWorld() = 0;
};