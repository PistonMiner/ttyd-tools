#include "elf2rel.h"

#include <elfio/elfio.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <tuple>
#include <deque>

std::map<std::string, uint32_t> loadSymbolMap(const std::string &filename)
{
	std::map<std::string, uint32_t> outputMap;

	std::ifstream inputStream(filename);
	for (std::string line; std::getline(inputStream, line); )
	{
		boost::trim_left(line);

		// Ignore comments
		if (line.size() == 0 || line.find_first_of("//") == 0)
		{
			continue;
		}

		size_t index = line.find_first_of(':');

		std::string name = line.substr(index + 1);
		boost::trim_left(name);

		uint32_t addr = strtoul(line.substr(0, index).c_str(), nullptr, 16);

		outputMap[name] = addr;
	}

	return outputMap;
}

void writeModuleHeader(std::vector<uint8_t> &buffer,
					   int id,
					   int sectionCount,
					   int sectionInfoOffset,
					   int totalBssSize,
					   int relocationOffset,
					   int importInfoOffset,
					   int importInfoSize,
					   int prologSection,
					   int epilogSection,
					   int unresolvedSection,
					   int prologOffset,
					   int epilogOffset,
					   int unresolvedOffset,
					   int maxAlign,
					   int maxBssAlign,
					   int fixedDataSize)
{
	save<uint32_t>(buffer, id);
	save<uint32_t>(buffer, 0); // prev link
	save<uint32_t>(buffer, 0); // next link
	save<uint32_t>(buffer, sectionCount);
	save<uint32_t>(buffer, sectionInfoOffset);
	save<uint32_t>(buffer, 0); // name offset
	save<uint32_t>(buffer, 0); // name size
	save<uint32_t>(buffer, 3); // version

	save<uint32_t>(buffer, totalBssSize);
	save<uint32_t>(buffer, relocationOffset);
	save<uint32_t>(buffer, importInfoOffset);
	save<uint32_t>(buffer, importInfoSize);
	save<uint8_t>(buffer, prologSection);
	save<uint8_t>(buffer, epilogSection);
	save<uint8_t>(buffer, unresolvedSection);
	save<uint8_t>(buffer, 0); // pad
	save<uint32_t>(buffer, prologOffset);
	save<uint32_t>(buffer, epilogOffset);
	save<uint32_t>(buffer, unresolvedOffset);
	save<uint32_t>(buffer, maxAlign);
	save<uint32_t>(buffer, maxBssAlign);
	save<uint32_t>(buffer, fixedDataSize);
}

void writeSectionInfo(std::vector<uint8_t> &buffer, int offset, int size)
{
	save<uint32_t>(buffer, offset);
	save<uint32_t>(buffer, size);
}

void writeImportInfo(std::vector<uint8_t> &buffer, int id, int offset)
{
	save<uint32_t>(buffer, id);
	save<uint32_t>(buffer, offset);
}

void writeRelocation(std::vector<uint8_t> &buffer, int offset, int type, int section, uint32_t addend)
{
	save<uint16_t>(buffer, offset);
	save<uint8_t>(buffer, type);
	save<uint8_t>(buffer, section);
	save<uint32_t>(buffer, addend);
}

const std::vector<std::string> cRelSectionMask = {
	".init",
	".text",
	".ctors",
	".dtors",
	".rodata",
	".data",
	".bss"
};

int main(int argc, char **argv)
{
	if (argc <= 2)
	{
		printf("Usage: %s <elf file> <symbol file>\n", argv[0]);
		return 1;
	}

	std::string elfFilename = argv[1];
	std::string lstFilename = argv[2];

	// Load input file
	ELFIO::elfio inputElf;
	if (!inputElf.load(elfFilename))
	{
		printf("Failed to load input file\n");
		return 1;
	}
	
	auto externalSymbolMap = loadSymbolMap(lstFilename);

	// Find special sections
	ELFIO::section *symSection = nullptr;
	std::vector<ELFIO::section *> relocationSections;
	for (const auto &section : inputElf.sections)
	{
		if (section->get_type() == SHT_SYMTAB)
		{
			symSection = section;
		}
		else if (section->get_type() == SHT_RELA)
		{
			relocationSections.emplace_back(section);
		}
	}

	// Symbol accessor
	ELFIO::symbol_section_accessor symbols(inputElf, symSection);

	// Find prolog, epilog and unresolved
	auto findSymbolSectionAndOffset = [&](const std::string &name, int &sectionIndex, int &offset)
	{
		ELFIO::Elf64_Addr addr;
		ELFIO::Elf_Xword size;
		unsigned char bind;
		unsigned char type;
		ELFIO::Elf_Half section_index;
		unsigned char other;
		for (int i = 0; i < symbols.get_symbols_num(); ++i)
		{
			std::string symbolName;
			if (symbols.get_symbol(static_cast<ELFIO::Elf_Xword>(i), symbolName, addr, size, bind, type, section_index, other))
			{
				if (symbolName == name)
				{
					sectionIndex = static_cast<int>(section_index);
					offset = static_cast<int>(addr);
					break;
				}
			}
		}
	};

	int prologSectionIndex = 0, prologOffset = 0;
	findSymbolSectionAndOffset("_prolog", prologSectionIndex, prologOffset);
	int epilogSectionIndex = 0, epilogOffset = 0;
	findSymbolSectionAndOffset("_epilog", epilogSectionIndex, epilogOffset);
	int unresolvedSectionIndex = 0, unresolvedOffset = 0;
	findSymbolSectionAndOffset("_unresolved", unresolvedSectionIndex, unresolvedOffset);

	std::vector<uint8_t> outputBuffer;
	// Dummy values for header until offsets are determined
	writeModuleHeader(outputBuffer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	int sectionInfoOffset = outputBuffer.size();
	for (int i = 0; i < inputElf.sections.size(); ++i)
	{
		writeSectionInfo(outputBuffer, 0, 0);
	}

	// Write sections
	std::vector<uint8_t> sectionInfoBuffer;
	std::vector<ELFIO::section *> writtenSections;
	int totalBssSize = 0;
	int maxAlign = 1;
	int maxBssAlign = 1;
	for (const auto &section : inputElf.sections)
	{
		// Should keep?
		if (std::find(cRelSectionMask.begin(),
					  cRelSectionMask.end(),
					  section->get_name()) != cRelSectionMask.end()
			&& section->get_size() != 0)
		{
			// BSS?
			if (section->get_type() == SHT_NOBITS)
			{
				// Update max alignment
				int align = static_cast<int>(section->get_addr_align());
				maxBssAlign = std::max(maxBssAlign, align);

				int size = static_cast<int>(section->get_size());
				totalBssSize += size;
				writeSectionInfo(sectionInfoBuffer, 0, size);
			}
			else
			{
				// Update max alignment
				int align = static_cast<int>(section->get_addr_align());
				maxAlign = std::max(maxAlign, align);

				// Write padding
				int requiredPadding = align - outputBuffer.size() % align;
				for (int i = 0; i < requiredPadding; ++i)
				{
					save<uint8_t>(outputBuffer, 0);
				}

				int offset = outputBuffer.size();
				// Mark executable sections
				if (section->get_flags() & SHF_EXECINSTR)
				{
					offset |= 1;
				}
				writeSectionInfo(sectionInfoBuffer, offset, static_cast<int>(section->get_size()));
				std::vector<uint8_t> sectionData(section->get_data(), section->get_data() + section->get_size());
				outputBuffer.insert(outputBuffer.end(), sectionData.begin(), sectionData.end());

				writtenSections.emplace_back(section);
			}
		}
		else
		{
			// Section was removed
			writeSectionInfo(sectionInfoBuffer, 0, 0);
		}
	}
	// Fill in section info in main buffer
	std::copy(sectionInfoBuffer.begin(), sectionInfoBuffer.end(), outputBuffer.begin() + sectionInfoOffset);

	// #todo-elf2rel: Make this accessible via program options, configured for TTYD right now
	int moduleID = 30;

	// Find all relocations
	struct Relocation
	{
		uint32_t moduleID; // target module
		uint32_t section;
		uint32_t offset;
		uint8_t targetSection;  // target symbol
		uint32_t addend;
		uint8_t type;
	};
	std::deque<Relocation> allRelocations;
	for (const auto &section : relocationSections)
	{
		int relocatedSectionIndex = section->get_info();
		ELFIO::section *relocatedSection = inputElf.sections[relocatedSectionIndex];
		// Only relocate sections that were written
		if (std::find(writtenSections.begin(), writtenSections.end(), relocatedSection) != writtenSections.end())
		{
			ELFIO::relocation_section_accessor relocations(inputElf, section);
			// #todo-elf2rel: Process relocations
			for (int i = 0; i < relocations.get_entries_num(); ++i)
			{
				ELFIO::Elf64_Addr offset;
				ELFIO::Elf_Word symbol;
				ELFIO::Elf_Word type;
				ELFIO::Elf_Sxword addend;
				relocations.get_entry(i, offset, symbol, type, addend);

				ELFIO::Elf_Xword size;
				unsigned char bind;
				unsigned char symbolType;
				ELFIO::Elf_Half sectionIndex;
				unsigned char other;
				std::string symbolName;
				ELFIO::Elf64_Addr symbolValue;
				if (!symbols.get_symbol(symbol, symbolName, symbolValue,
										size, bind, symbolType, sectionIndex, other))
				{
					printf("Unable to find symbol %u in symbol table!\n", static_cast<uint32_t>(symbol));
					return 1;
				}

				// Add relocation to list
				bool resolved = false;
				Relocation rel;
				rel.section = relocatedSectionIndex;
				rel.offset = static_cast<uint32_t>(offset);
				rel.type = type;
				if (sectionIndex)
				{
					// Self-relocation
					resolved = true;

					rel.moduleID = moduleID;
					rel.targetSection = static_cast<uint8_t>(sectionIndex);
					rel.addend = static_cast<uint32_t>(addend + symbolValue);
				}
				else
				{
					// Symbol is unknown, check if it's an external known symbol
					auto it = externalSymbolMap.find(symbolName);
					if (it != externalSymbolMap.end())
					{
						// Known external!
						resolved = true;

						rel.moduleID = 0;
						rel.targetSection = 0; // #todo-elf2rel: Check if this is important
						rel.addend = it->second;
					}
				}

				if (resolved)
				{
					allRelocations.emplace_back(rel);
				}
				else
				{
					printf("Unresolved external symbol '%s'\n", symbolName.c_str());
				}
			}
		}
	}

	// Sort relocations
	std::sort(allRelocations.begin(), allRelocations.end(),
			  [](const Relocation &left, const Relocation &right)
	{
		return std::tuple<uint32_t, uint32_t, uint32_t>(left.moduleID, left.section, left.offset)
			   < std::tuple<uint32_t, uint32_t, uint32_t>(right.moduleID, right.section, right.offset);
	});

	// Count modules
	int importCount = 0;
	int lastModuleID = -1;
	for (const auto &rel : allRelocations)
	{
		if (lastModuleID != rel.moduleID)
		{
			lastModuleID = rel.moduleID;
			++importCount;
		}
	}

	// Write padding for imports
	int requiredPadding = 8 - outputBuffer.size() % 8;
	for (int i = 0; i < requiredPadding; ++i)
	{
		save<uint8_t>(outputBuffer, 0);
	}

	// Write dummy imports
	int importInfoOffset = outputBuffer.size();
	for (int i = 0; i < importCount; ++i)
	{
		writeImportInfo(outputBuffer, 0, 0);
	}

	// Write out relocations
	int relocationOffset = outputBuffer.size();

	std::vector<uint8_t> importInfoBuffer;
	int currentModuleID = -1;
	int currentSectionIndex = -1;
	int currentOffset = 0;
	while (!allRelocations.empty())
	{
		Relocation nextRel = allRelocations.front();
		allRelocations.pop_front();

		// Ignore R_PPC_NONE
		if (nextRel.type == R_PPC_NONE)
			continue;

		// Change module if necessary
		if (currentModuleID != nextRel.moduleID)
		{
			// Not first module?
			if (currentModuleID != -1)
			{
				writeRelocation(outputBuffer, 0, R_DOLPHIN_END, 0, 0);
			}

			currentModuleID = nextRel.moduleID;
			currentSectionIndex = -1;
			writeImportInfo(importInfoBuffer, currentModuleID, outputBuffer.size());
		}

		// Change section if necessary
		if (currentSectionIndex != nextRel.section)
		{
			currentSectionIndex = nextRel.section;
			currentOffset = 0;
			writeRelocation(outputBuffer, 0, R_DOLPHIN_SECTION, currentSectionIndex, 0);
		}

		// Get into range of the target
		int targetDelta = nextRel.offset - currentOffset;
		while (targetDelta > 0xFFFF)
		{
			writeRelocation(outputBuffer, 0, R_DOLPHIN_NOP, 0, 0);
			targetDelta -= 0xFFFF;
		}
		// #todo-elf2rel: Resolve potential self-relocations at build time here
		// #todo-elf2rel: Add runtime unresolved symbol handling here
		writeRelocation(outputBuffer, targetDelta, nextRel.type, nextRel.targetSection, nextRel.addend);
		currentOffset = nextRel.offset;
	}
	writeRelocation(outputBuffer, 0, R_DOLPHIN_END, 0, 0);

	// Write final import infos
	int importInfoSize = importInfoBuffer.size();
	std::copy(importInfoBuffer.begin(), importInfoBuffer.end(), outputBuffer.begin() + importInfoOffset);
		
	// Write final header
	std::vector<uint8_t> headerBuffer;
	writeModuleHeader(headerBuffer,
					  moduleID,
					  inputElf.sections.size(),
					  sectionInfoOffset,
					  totalBssSize,
					  relocationOffset,
					  importInfoOffset,
					  importInfoSize,
					  prologSectionIndex, epilogSectionIndex, unresolvedSectionIndex,
					  prologOffset, epilogOffset, unresolvedOffset,
					  maxAlign,
					  maxBssAlign,
					  relocationOffset);
	std::copy(headerBuffer.begin(), headerBuffer.end(), outputBuffer.begin());

	// Write final REL file
	std::string relFilename = elfFilename.substr(0, elfFilename.find_last_of('.')) + ".rel";
	std::ofstream outputStream(relFilename, std::ios::binary);
	outputStream.write(reinterpret_cast<const char *>(outputBuffer.data()), outputBuffer.size());
	
	return 0;
}