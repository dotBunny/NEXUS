// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_INTERFACE_INVOKE_METHOD(InterfaceName, MethodName, EventName) \
	static void Invoke##MethodName(UObject* BaseObject) \
	{ \
		if (BaseObject->Implements<U##InterfaceName>()) \
		{ \
			I##InterfaceName* InterfaceObject = Cast<I##InterfaceName>(BaseObject); \
			if (InterfaceObject != nullptr) \
			{ \
				InterfaceObject->##MethodName(BaseObject); \
			} \
			else \
			{ \
				Execute_##EventName(BaseObject); \
			} \
		} \
	}
#define N_INTERFACE_INVOKE_METHOD_OneParam(InterfaceName, MethodName, EventName, ParamType, ParamName) \
	static void Invoke##MethodName(UObject* BaseObject, ParamType ParamName) \
	{ \
		if (BaseObject->Implements<U##InterfaceName>()) \
		{ \
			I##InterfaceName* InterfaceObject = Cast<I##InterfaceName>(BaseObject); \
			if (InterfaceObject != nullptr) \
			{ \
				InterfaceObject->##MethodName(BaseObject, ParamName); \
			} \
			else \
			{ \
				Execute_##EventName(BaseObject, ParamName); \
			} \
		} \
	}
#define N_INTERFACE_INVOKE_METHOD_TwoParams(InterfaceName, MethodName, EventName, ParamOneType, ParamOneName, ParamTwoType, ParamTwoName) \
	static void Invoke##MethodName(UObject* BaseObject, ParamOneType ParamOneName, ParamTwoType ParamTwoName) \
	{ \
		if (BaseObject->Implements<U##InterfaceName>()) \
		{ \
			I##InterfaceName* InterfaceObject = Cast<I##InterfaceName>(BaseObject); \
			if (InterfaceObject != nullptr) \
			{ \
				InterfaceObject->##MethodName(BaseObject, ParamOneName, ParamTwoName); \
			} \
			else \
			{ \
				Execute_##EventName(BaseObject, ParamOneName, ParamTwoName); \
			} \
		} \
	}