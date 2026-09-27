// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Internationalization/Regex.h"

/**
 * Native string helpers.
 * @see <a href="https://nexus-framework.com/docs/core/types/string-utils/">FNStringUtils</a>
 */
class FNStringUtils
{
public:
	/**
	 * Checks whether Email is shaped like an ordinary email address: local part, '@', domain, and a top-level
	 * domain of at least two letters.
	 * @param Email The string to test.
	 * @return true when the whole string matches.
	 * @note A pragmatic pattern rather than full RFC 5322: quoted local parts, IP-literal domains and
	 *       internationalized addresses are rejected. Use it to catch typos, not to decide deliverability.
	 */
	static bool IsValidEmail(const FString& Email)
	{
		// Basic RFC 5322 compliant pattern for standard email validation
		const FRegexPattern Pattern(TEXT(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)" ));
		FRegexMatcher Matcher(Pattern, Email);
		return Matcher.FindNext();
	}
};
