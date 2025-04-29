// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_IMPLEMENT_ASSET_FACTORY(Type) \
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, \
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override \
	{ \
		check(InClass->IsChildOf(Type::StaticClass())); \
		return NewObject<Type>(InParent, InClass, InName, Flags); \
	}