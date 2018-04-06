#include "patch.h"

#include <cstdint>

namespace mod::patch {

void writeBranch(void *ptr, void *destination)
{
	uint32_t delta = reinterpret_cast<uint32_t>(destination) - reinterpret_cast<uint32_t>(ptr);
	uint32_t value = 0x48000000;
	value |= (delta & 0x03FFFFFC);
	
	uint32_t *p = reinterpret_cast<uint32_t *>(ptr);
	*p = value;
}

}