// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Generates a static Invoke<MethodName> helper that dispatches to the native interface impl when
 * available, or falls back to the Blueprint-generated Execute_<EventName> path.
 *
 * Lets callers treat Blueprint-implemented and native-implemented interface methods uniformly
 * without having to write the Implements<>/Cast<>/Execute_ plumbing at each call site.
 *
 * @param InterfaceName Bare interface name (without the U/I prefix).
 * @param MethodName Native method name on I<InterfaceName>.
 * @param EventName Blueprint-facing function name used with Execute_<EventName>.
 */
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
/**
 * One-parameter variant of N_INTERFACE_INVOKE_METHOD.
 * @param ParamType Parameter type forwarded to the interface method and Execute_ fallback.
 * @param ParamName Parameter name in the generated dispatcher.
 */
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
/**
 * Two-parameter variant of N_INTERFACE_INVOKE_METHOD.
 * @param ParamOneType First parameter type.
 * @param ParamOneName First parameter name.
 * @param ParamTwoType Second parameter type.
 * @param ParamTwoName Second parameter name.
 */
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