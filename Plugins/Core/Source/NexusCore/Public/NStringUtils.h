// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class FNStringUtils
{
public:
	static bool IsValidEmail(const FString& Email)
	{
		// Basic RFC 5322 compliant pattern for standard email validation
		const FRegexPattern Pattern(TEXT(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)" ));
		FRegexMatcher Matcher(Pattern, Email);
		return Matcher.FindNext();
	}
};
