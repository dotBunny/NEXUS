// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNActorPoolsEditorCommands
{
public:
	static void AddMenuEntries();
	static void RemoveMenuEntries();
	
	static void AddActorPoolMethods(const UObject* EditedObject);
};
