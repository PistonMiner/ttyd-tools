// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Linus S. (aka PistonMiner)

#include "elf2rel.h"

#include <elfio/elfio.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <tuple>
#include <deque>

struct SymbolLocation
{
	uint32_t moduleId; // 0 means dol
	uint32_t targetSection; // OSLink ignores for dol
	uint32_t addr;
};

std::map<std::string, SymbolLocation> loadSymbolMap(const std::string &filename)
{
	std::map<std::string, SymbolLocation> outputMap;

	std::ifstream inputStream(filename);
	for (std::string line; std::getline(inputStream, line); )
	{
		boost::trim_left(line);

		// Ignore comments
		if (line.size() == 0 || line.find_first_of("//") == 0)
		{
			continue;
		}

		// dol symbols:    addr:symbolName
		// rel symbols:    module,section,offset:symbolName
		
		uint32_t moduleId, sectionId, addr;
		std::string name;

		// Find name after colon separator, always present
		size_t colonIndex = line.find_first_of(':');
		name = line.substr(colonIndex + 1);
		boost::trim_left(name);

		// Handle comma separators, if present
		// Commas can be included in mangled symbol names, so this has to be limited to before the name
		std::string locInfo = line.substr(0, colonIndex);
		size_t commaIndex1 = line.find_first_of(',');
		if (commaIndex1 == std::string::npos)
		{
			moduleId = 0;
			sectionId = 0;
			addr = std::stoul(locInfo, nullptr, 16);
		}
		else
		{
			size_t commaIndex2 = line.find_first_of(',', commaIndex1 + 1);

			std::string moduleIdStr = locInfo.substr(0, commaIndex1);
			if (moduleIdStr.substr(0, 2) == "0x")
				moduleId = std::stoul(moduleIdStr.substr(2), nullptr, 16);
			else
				moduleId = std::stoul(moduleIdStr, nullptr, 10);

			std::string sectionIdStr = locInfo.substr(commaIndex1 + 1, commaIndex2 - commaIndex1);
			if (sectionIdStr.substr(0, 2) == "0x")
				sectionId = std::stoul(sectionIdStr.substr(2), nullptr, 16);
			else
				sectionId = std::stoul(sectionIdStr, nullptr, 10);

			std::string addrStr = locInfo.substr(commaIndex2 + 1);
			addr = std::stoul(addrStr, nullptr, 16);
		}

		outputMap[name] = { moduleId, sectionId, addr };
	}

	return outputMap;
}

void writeModuleHeader(std::vector<uint8_t> &buffer,
					   int version,
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
	save<uint32_t>(buffer, version); // version

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
	if (version >= 2)
	{
		save<uint32_t>(buffer, maxAlign);
		save<uint32_t>(buffer, maxBssAlign);
	}
	if (version >= 3)
	{
		save<uint32_t>(buffer, fixedDataSize);
	}
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
	std::string elfFilename;
	std::string lstFilename;
	std::string relFilename = "";
	int moduleID = 33;
	int relVersion = 3;

	{
		namespace po = boost::program_options;

		po::options_description description("Options");
		description.add_options()
			("help", "Print help message")
			("input-file,i", po::value(&elfFilename), "Input ELF filename (required)")
			("symbol-file,s", po::value(&lstFilename), "Input symbol file name (required)")
			("output-file,o", po::value(&relFilename), "Output REL filename")
			("rel-id", po::value(&moduleID)->default_value(0x1000), "REL file ID")
			("rel-version", po::value(&relVersion)->default_value(3), "REL file format version (1, 2, 3)");

		po::positional_options_description positionals;
		positionals.add("input-file", -1);

		po::variables_map varMap;
		po::store(
			po::command_line_parser(argc, argv)
				.options(description)
				.positional(positionals)
				.run(),
			varMap
		);
		po::notify(varMap);

		if (varMap.count("help")
			|| varMap.count("input-file") != 1
			|| varMap.count("symbol-file") != 1
			|| relVersion < 1
			|| relVersion > 3)
		{
			std::cout << description << "\n";
			return 1;
		}
	}

	if (relFilename == "")
	{
		relFilename = elfFilename.substr(0, elfFilename.find_last_of('.')) + ".rel";
	}
	
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
	writeModuleHeader(outputBuffer, relVersion, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	int sectionInfoOffset = outputBuffer.size();
	for (int i = 0; i < inputElf.sections.size(); ++i)
	{
		writeSectionInfo(outputBuffer, 0, 0);
	}

	// Write sections
	std::vector<uint8_t> sectionInfoBuffer;
	std::map<ELFIO::section *, int> writtenSections;
	int totalBssSize = 0;
	int maxAlign = 2;
	int maxBssAlign = 2;
	for (const auto &section : inputElf.sections)
	{
		// Should keep?
		if (std::find_if(cRelSectionMask.begin(),
						  cRelSectionMask.end(),
						  [&](const std::string &val)
		{
			return val == section->get_name()
				   || section->get_name().find(val + ".") == 0;
		}) != cRelSectionMask.end())
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
				// Update max alignment (minimum 2, low offset bit is used for exec flag)
				int align = std::max(static_cast<int>(section->get_addr_align()), 2);
				maxAlign = std::max(maxAlign, align);

				// Write padding
				int requiredPadding = ((outputBuffer.size() + align - 1) & ~(align - 1)) - outputBuffer.size();
				for (int i = 0; i < requiredPadding; ++i)
				{
					save<uint8_t>(outputBuffer, 0);
				}

				int offset = outputBuffer.size();

				int encodedOffset = offset;
				// Mark executable sections
				if (section->get_flags() & SHF_EXECINSTR)
				{
					encodedOffset |= 1;
				}
				writeSectionInfo(sectionInfoBuffer, encodedOffset, static_cast<int>(section->get_size()));
				std::vector<uint8_t> sectionData(section->get_data(), section->get_data() + section->get_size());
				outputBuffer.insert(outputBuffer.end(), sectionData.begin(), sectionData.end());

				writtenSections[section] = offset;
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
		if (writtenSections.find(relocatedSection) != writtenSections.end())
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

				// Ignore R_PPC_NONE
				if (type == R_PPC_NONE)
					continue;

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

					ELFIO::section *targetSection = inputElf.sections[rel.targetSection];
					if (writtenSections.find(targetSection) == writtenSections.end() && targetSection->get_type() != SHT_NOBITS)
					{
						printf("Relocation from section '%s' offset %llx against symbol '%s' in unwritten section '%s'\n",
							   relocatedSection->get_name().c_str(),
							   offset,
							   symbolName.c_str(),
							   targetSection->get_name().c_str());
					}
				}
				else
				{
					// Symbol is unknown, check if it's an external known symbol
					auto it = externalSymbolMap.find(symbolName);
					if (it != externalSymbolMap.end())
					{
						// Known external!
						resolved = true;
						rel.moduleID = it->second.moduleId;
						rel.targetSection = it->second.targetSection;
						rel.addend = static_cast<uint32_t>(addend + it->second.addr);
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
		
		// Resolve early if possible
		if (nextRel.moduleID == moduleID && (nextRel.type == R_PPC_REL24 || nextRel.type == R_PPC_REL32))
		{
			int offset = writtenSections.at(inputElf.sections[nextRel.section]) + nextRel.offset;
			int delta = writtenSections.at(inputElf.sections[nextRel.targetSection]) + nextRel.addend - offset;
			std::vector<uint8_t> instructionBuffer(outputBuffer.begin() + offset, outputBuffer.begin() + offset + 4);
			uint32_t patchedData;
			load(instructionBuffer, patchedData);
			
			if (nextRel.type == R_PPC_REL24)
			{
				patchedData |= (delta & 0x03FFFFFC);
			}
			else if (nextRel.type == R_PPC_REL32)
			{
				patchedData = delta;
			}
			
			save(instructionBuffer, patchedData);
			std::copy(instructionBuffer.begin(), instructionBuffer.end(), outputBuffer.begin() + offset);

			continue;
		}

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
			writeRelocation(outputBuffer, 0xFFFF, R_DOLPHIN_NOP, 0, 0);
			targetDelta -= 0xFFFF;
		}
		
		// #todo-elf2rel: Add runtime unresolved symbol handling here
		// At this point, only symbols that OSLink can handle should remain
		switch (nextRel.type)
		{
		case R_PPC_NONE:
		case R_PPC_ADDR32:
		case R_PPC_ADDR24:
		case R_PPC_ADDR16:
		case R_PPC_ADDR16_LO:
		case R_PPC_ADDR16_HI:
		case R_PPC_ADDR16_HA:
		case R_PPC_ADDR14:
		case R_PPC_ADDR14_BRTAKEN:
		case R_PPC_ADDR14_BRNKTAKEN:
		case R_PPC_REL24:
		case R_DOLPHIN_NOP:
		case R_DOLPHIN_SECTION:
		case R_DOLPHIN_END:
			break;
		default:
			printf("Unsupported relocation type %d\n", nextRel.type);
			break;
		}

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
					  relVersion,
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
	std::ofstream outputStream(relFilename, std::ios::binary);
	outputStream.write(reinterpret_cast<const char *>(outputBuffer.data()), outputBuffer.size());
	
	return 0;
}
