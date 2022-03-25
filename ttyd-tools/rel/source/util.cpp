#include "util.h"

#include <cstring>
#include <cstdint>

// TTYD doesn't use this so we reimplement it.
// TODO: Not the greatest solution in the long run
char *strstr(const char *container, const char *pattern)
{
	int pattern_len = strlen(pattern);
	for (const char *start = container; *start; ++start)
	{
		bool mismatch = false;
		for (int i = 0; i < pattern_len; ++i)
		{
			if (!container[i] || start[i] != pattern[i])
			{
				mismatch = true;
				break;
			}
		}
		if (!mismatch)
		{
			// Cast away the const because C demands it
			return (char *)start;
		}
	}
	return nullptr;
}