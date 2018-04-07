#pragma once

#include <vector>

enum RelRelocationType
{
	R_PPC_NONE = 0,
	R_PPC_ADDR32,
	R_PPC_ADDR24,
	R_PPC_ADDR16,
	R_PPC_ADDR16_LO,
	R_PPC_ADDR16_HI,
	R_PPC_ADDR16_HA,
	R_PPC_ADDR14,
	R_PPC_ADDR14_BRTAKEN,
	R_PPC_ADDR14_BRNKTAKEN,
	R_PPC_REL24,
	R_PPC_REL14,

	R_PPC_REL32 = 26,

	R_DOLPHIN_NOP = 201,
	R_DOLPHIN_SECTION,
	R_DOLPHIN_END,
};

template<typename T>
void save(std::vector<uint8_t> &buffer, const T &value)
{
	for (size_t i = sizeof(T); i > 0; --i)
	{
		buffer.emplace_back(static_cast<uint8_t>((value >> (i - 1) * 8) & 0xFF));
	}
}

template<typename T>
void load(std::vector<uint8_t> &buffer, T &value)
{
	value = 0;
	for (size_t i = sizeof(T); i > 0; --i)
	{
		value |= static_cast<T>(buffer.front()) << ((i - 1) * 8);
		buffer.erase(buffer.begin());
	}
}