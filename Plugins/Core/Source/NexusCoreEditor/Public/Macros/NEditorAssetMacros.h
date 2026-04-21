// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Implements the standard UFactory::FactoryCreateNew override that spawns a new Type instance.
 *
 * Place this inside a UFactory subclass header; it creates the override body so the factory can
 * construct the target asset without boilerplate. Asserts that InClass derives from Type.
 * @param Type The UObject subclass produced by the factory.
 */
#define N_IMPLEMENT_ASSET_FACTORY(Type) \
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, \
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override \
	{ \
		check(InClass->IsChildOf(Type::StaticClass())); \
		return NewObject<Type>(InParent, InClass, InName, Flags); \
	}