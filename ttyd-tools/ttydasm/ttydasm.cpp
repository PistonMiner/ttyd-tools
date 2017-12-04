#include "ttydasm.h"

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <queue>

#include "platform.h"

boost::program_options::variables_map gVarMap;

std::vector<std::string> argStartOffsetStrings;
std::vector<std::string> argStartAddressStrings;
std::vector<std::string> argStartSymbolStrings;
std::string argInputFileName;
std::string argImageBaseString;
std::vector<std::string> argSymbolFileNames;
bool argCrossRefScripts;

unsigned char *gFileData;
uint32_t gFileSize;
uint32_t gCurrentAddress = 0;
uint32_t gBaseAddress;

std::vector<uint32_t> gDisassemblyList;

const char *cIndentLevel = "  ";

std::map<uint32_t, std::string> gSymbolMap;

namespace ExpressionZones
{
const int cZoneExtent = 10000000;
const int cAddrBase = -250000000;
const int cFloatBase = -230000000;
const int cUFBase = -210000000;
const int cUWBase = -190000000;
const int cGSWBase = -170000000;
const int cLSWBase = -150000000;
const int cGSWFBase = -130000000;
const int cLSWFBase = -110000000;
const int cGFBase = -90000000;
const int cLFBase = -70000000;
const int cGWBase = -50000000;
const int cLWBase = -30000000;
}

template<typename... fmt_args>
std::string formatString(char *format, fmt_args... args)
{
	static char sFormatBuf[512];
	snprintf(sFormatBuf, sizeof(sFormatBuf), format, args...);
	return std::string(sFormatBuf);
}

inline bool isAddrLoaded(uint32_t addr)
{
	return addr >= gBaseAddress && addr < gBaseAddress + gFileSize;
}

void *loadFile(const std::string &filename, uint32_t *filesize = nullptr, const char *mode = "rb")
{
	FILE *file = fopen(filename.c_str(), mode);
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	void *mem = new unsigned char[size];
	fseek(file, 0, SEEK_SET);
	fread(mem, 1, size, file);
	fclose(file);

	if (filesize)
		*filesize = size;

	return mem;
}

std::string lookupSymbol(uint32_t addr)
{
	auto it = gSymbolMap.find(addr);

	if (it == gSymbolMap.end())
		return "";

	return it->second;
}

void loadSymbolMap(const std::string &filename)
{
	uint32_t filesize;
	char *data = static_cast<char *>(loadFile(filename, &filesize, "r"));
	std::string data_string(data, filesize);
	std::stringstream data_stream(data_string);

	std::string line = "";
	while (std::getline(data_stream, line))
	{
		size_t index = line.find_first_of(':');

		std::string name = line.substr(index + 1);
		boost::trim_left(name);

		uint32_t addr = strtoul(line.substr(0, index).c_str(), nullptr, 16);

		gSymbolMap[addr] = name;
	}

	delete data;
}

enum ScriptOpcode
{
	OP_InternalFetch,
	OP_ScriptEnd,
	OP_Return,
	OP_Label,
	OP_Goto,
	OP_LoopBegin,
	OP_LoopIterate,
	OP_LoopBreak,
	OP_LoopContinue,
	OP_WaitFrames,
	OP_WaitMS,
	OP_WaitUntil,
	OP_IfStringEqual,
	OP_IfStringNotEqual,
	OP_IfStringLess,
	OP_IfStringGreater,
	OP_IfStringLessEqual,
	OP_IfStringGreaterEqual,
	OP_IfFloatEqual,
	OP_IfFloatNotEqual,
	OP_IfFloatLess,
	OP_IfFloatGreater,
	OP_IfFloatLessEqual,
	OP_IfFloatGreaterEqual,
	OP_IfIntEqual,
	OP_IfIntNotEqual,
	OP_IfIntLess,
	OP_IfIntGreater,
	OP_IfIntLessEqual,
	OP_IfIntGreaterEqual,
	OP_IfBitsSet,
	OP_IfBitsClear,
	OP_Else,
	OP_EndIf,
	OP_SwitchExpr,
	OP_SwitchRaw,
	OP_CaseIntEqual,
	OP_CaseIntNotEqual,
	OP_CaseIntLess,
	OP_CaseIntGreater,
	OP_CaseIntLessEqual,
	OP_CaseIntGreaterEqual,
	OP_CaseDefault,
	OP_CaseIntEqualAny,
	OP_CaseIntNotEqualAll,
	OP_CaseBitsSet,
	OP_EndMultiCase,
	OP_CaseIntRange,
	OP_SwitchBreak,
	OP_EndSwitch,
	OP_SetExprIntToExprInt,
	OP_SetExprIntToRaw,
	OP_SetExprFloatToExprFloat,
	OP_AddInt,
	OP_SubtractInt,
	OP_MultiplyInt,
	OP_DivideInt,
	OP_ModuloInt,
	OP_AddFloat,
	OP_SubtractFloat,
	OP_MultiplyFloat,
	OP_DivideFloat,
	OP_MemOpSetBaseInt,
	OP_MemOpReadInt,
	OP_MemOpReadInt2,
	OP_MemOpReadInt3,
	OP_MemOpReadInt4,
	OP_MemOpReadIntIndexed,
	OP_MemOpSetBaseFloat,
	OP_MemOpReadFloat,
	OP_MemOpReadFloat2,
	OP_MemOpReadFloat3,
	OP_MemOpReadFloat4,
	OP_MemOpReadFloatIndexed,
	OP_SetUserWordBase,
	OP_SetUserFlagBase,
	OP_AllocateUserWordBase,
	OP_AndExpr,
	OP_AndRaw,
	OP_OrExpr,
	OP_OrRaw,
	OP_ConvertMSToFrames,
	OP_ConvertFramesToMS,
	OP_StoreIntToPtr,
	OP_StoreFloatToPtr,
	OP_LoadIntFromPtr,
	OP_LoadFloatFromPtr,
	OP_StoreIntToPtrExpr,
	OP_StoreFloatToPtrExpr,
	OP_LoadIntFromPtrExpr,
	OP_LoadFloatFromPtrExpr,
	OP_CallCppSync,
	OP_CallScriptAsync,
	OP_CallScriptAsyncSaveTID,
	OP_CallScriptSync,
	OP_TerminateThread,
	OP_Jump,
	OP_SetThreadPriority,
	OP_SetThreadTimeQuantum,
	OP_SetThreadTypeMask,
	OP_ThreadSuspendTypes,
	OP_ThreadResumeTypes,
	OP_ThreadSuspendTypesOther,
	OP_ThreadResumeTypesOther,
	OP_ThreadSuspendTID,
	OP_ThreadResumeTID,
	OP_CheckThreadRunning,
	OP_ThreadStart,
	OP_ThreadStartSaveTID,
	OP_ThreadEnd,
	OP_ThreadChildStart,
	OP_ThreadChildStartSaveTID,
	OP_ThreadChildEnd,
	OP_DebugOutputString,
	OP_DebugUnk1,
	OP_DebugExprToString,
	OP_DebugUnk2,
	OP_DebugUnk3
};

enum class ExpressionType
{
	Address,
	Float,
	UF,
	UW,
	GSW,
	LSW,
	GSWF,
	LSWF,
	GF,
	LF,
	GW,
	LW,
	Immediate,
};

ExpressionType categorizeExpr(uint32_t expr)
{
	using namespace ExpressionZones;

#define IS_EXPR_TYPE(name) \
	(val >= c##name##Base && val <= c##name##Base + cZoneExtent)

	const int32_t &val = *reinterpret_cast<int32_t *>(&expr);
	if (val <= cAddrBase)
	{
		return ExpressionType::Address;
	}
	else if (val < cUFBase)
	{
		return ExpressionType::Float;
	}
	else if (IS_EXPR_TYPE(UF))
	{
		return ExpressionType::UF;
	}
	else if (IS_EXPR_TYPE(UW))
	{
		return ExpressionType::UW;
	}
	else if (IS_EXPR_TYPE(GSW))
	{
		return ExpressionType::GSW;
	}
	else if (IS_EXPR_TYPE(LSW))
	{
		return ExpressionType::LSW;
	}
	else if (IS_EXPR_TYPE(GSWF))
	{
		return ExpressionType::GSWF;
	}
	else if (IS_EXPR_TYPE(LSWF))
	{
		return ExpressionType::LSWF;
	}
	else if (IS_EXPR_TYPE(GF))
	{
		return ExpressionType::GF;
	}
	else if (IS_EXPR_TYPE(LF))
	{
		return ExpressionType::LF;
	}
	else if (IS_EXPR_TYPE(GW))
	{
		return ExpressionType::GW;
	}
	else if (IS_EXPR_TYPE(LW))
	{
		return ExpressionType::LW;
	}
	else
	{
		return ExpressionType::Immediate;
	}
#undef IS_EXPR_TYPE
}

enum class NumericalFormat
{
	Decimal,
	Hex,
};

std::string exprToString(uint32_t expr, NumericalFormat fmt = NumericalFormat::Decimal)
{
	using namespace ExpressionZones;

	ExpressionType type = categorizeExpr(expr);

	const int32_t &val = *reinterpret_cast<int32_t *>(&expr);
	if (type == ExpressionType::Address)
	{
		std::string symbolName = lookupSymbol(expr);
		return symbolName != "" ? formatString("[%s]", symbolName.c_str()) : formatString("[%08X]", val);
	}
	else if (type == ExpressionType::Float)
	{
		return formatString("%4.2f", (val - cFloatBase) / 1024.f);
	}
	else if (type == ExpressionType::UF)
	{
		return formatString("UF(%d)", (val - cUFBase));
	}
	else if (type == ExpressionType::UW)
	{
		return formatString("UW(%d)", (val - cUWBase));
	}
	else if (type == ExpressionType::GSW)
	{
		return formatString("GSW(%d)", (val - cGSWBase));
	}
	else if (type == ExpressionType::LSW)
	{
		return formatString("LSW(%d)", (val - cLSWBase));
	}
	else if (type == ExpressionType::GSWF)
	{
		return formatString("GSWF(%d)", (val - cGSWFBase));
	}
	else if (type == ExpressionType::LSWF)
	{
		return formatString("LSWF(%d)", (val - cLSWFBase));
	}
	else if (type == ExpressionType::GF)
	{
		return formatString("GF(%d)", (val - cGFBase));
	}
	else if (type == ExpressionType::LF)
	{
		return formatString("LF(%d)", (val - cLFBase));
	}
	else if (type == ExpressionType::GW)
	{
		return formatString("GW(%d)", (val - cGWBase));
	}
	else if (type == ExpressionType::LW)
	{
		return formatString("LW(%d)", (val - cLWBase));
	}
	else /* if (type == ExpressionType::Immediate */
	{
		if (fmt == NumericalFormat::Hex)
		{
			return formatString("0x%X", val);
		}
		else
		{
			return formatString("%d", val);
		}
	}
}

std::string disassembleOpcode(uint32_t &address, std::string &indent, bool *done = nullptr)
{
	auto readLong = [&](uint32_t address)
	{
		return _byteswap_ulong(*reinterpret_cast<uint32_t *>(gFileData + address - gBaseAddress));
	};
	auto readParm = [&](uint32_t argIndex)
	{
		return readLong(address + argIndex * sizeof(uint32_t));
	};
	auto addIndent = [&]()
	{
		indent += cIndentLevel;
	};
	auto removeIndent = [&]()
	{
		indent = indent.substr(0, indent.size() - strlen(cIndentLevel));
	};

	uint32_t header = readLong(address);
	address += sizeof(uint32_t);

	uint16_t opcode = header & 0xFFFF;
	uint16_t param_count = (header >> 16 & 0xFFFF);

#define PRINT_ARGS \
	for (uint32_t i = 0; i < param_count; ++i) \
	{ \
		out += " " + exprToString(readParm(i)); \
	}

#define PASSTHROUGH(value, name) \
	case value: \
		out = indent + name; \
		PRINT_ARGS; \
		break;

#define INDENT_IN(value, name) \
	case value: \
		out = indent + name; \
		PRINT_ARGS; \
		addIndent(); \
		break;

#define INDENT_OUT(value, name) \
	case value: \
		removeIndent(); \
		out = indent + name; \
		PRINT_ARGS; \
		break;
	
#define INDENT_OUTIN(value, name) \
	case value: \
		removeIndent(); \
		out = indent + name; \
		addIndent(); \
		PRINT_ARGS; \
		break;

	// Mnemonic handling
	std::string out;
	switch (opcode)
	{
	case OP_ScriptEnd:
		out = indent + "end";
		if (done)
		{
			*done = true;
		}
		break;
	PASSTHROUGH(OP_Return,				"return");
	case OP_Label:
		out = formatString("%d:", readParm(0));
		break;
	PASSTHROUGH(OP_Goto,				"goto");
	INDENT_IN(OP_LoopBegin,				"loop");
	INDENT_OUT(OP_LoopIterate,			"end_loop");
	PASSTHROUGH(OP_LoopBreak,			"loop_break");
	PASSTHROUGH(OP_LoopContinue,		"loop_continue");
	PASSTHROUGH(OP_WaitFrames,			"wait_frames");
	PASSTHROUGH(OP_WaitMS,				"wait_ms");
	PASSTHROUGH(OP_WaitUntil,			"wait_until");
	INDENT_IN(OP_IfStringEqual,			"if_string_eq");
	INDENT_IN(OP_IfStringNotEqual,		"if_string_ne");
	INDENT_IN(OP_IfStringLess,			"if_string_lt");
	INDENT_IN(OP_IfStringGreater,		"if_string_gt");
	INDENT_IN(OP_IfStringLessEqual,		"if_string_le");
	INDENT_IN(OP_IfStringGreaterEqual,	"if_string_ge");
	INDENT_IN(OP_IfFloatEqual,			"if_float_eq");
	INDENT_IN(OP_IfFloatNotEqual,		"if_float_ne");
	INDENT_IN(OP_IfFloatLess,			"if_float_lt");
	INDENT_IN(OP_IfFloatGreater,		"if_float_gt");
	INDENT_IN(OP_IfFloatLessEqual,		"if_float_le");
	INDENT_IN(OP_IfFloatGreaterEqual,	"if_float_ge");
	INDENT_IN(OP_IfIntEqual,			"if_int_eq");
	INDENT_IN(OP_IfIntNotEqual,			"if_int_ne");
	INDENT_IN(OP_IfIntLess,				"if_int_lt");
	INDENT_IN(OP_IfIntGreater,			"if_int_gt");
	INDENT_IN(OP_IfIntLessEqual,		"if_int_le");
	INDENT_IN(OP_IfIntGreaterEqual,		"if_int_ge");
	INDENT_IN(OP_IfBitsSet,				"if_bits_set");
	INDENT_IN(OP_IfBitsClear,			"if_bits_clear");
	INDENT_OUTIN(OP_Else,				"else");
	INDENT_OUT(OP_EndIf,				"endif");
	case OP_SwitchExpr:
		out = indent + "switchi";
		PRINT_ARGS;
		addIndent();
		addIndent();
		break;
	case OP_SwitchRaw:
		out = indent + "switchr";
		PRINT_ARGS;
		addIndent();
		addIndent();
		break;
	INDENT_OUTIN(OP_CaseIntEqual,		"case_int_eq");
	INDENT_OUTIN(OP_CaseIntNotEqual,	"case_int_ne");
	INDENT_OUTIN(OP_CaseIntLess,		"case_int_lt");
	INDENT_OUTIN(OP_CaseIntGreater,		"case_int_gt");
	INDENT_OUTIN(OP_CaseIntLessEqual,	"case_int_le");
	INDENT_OUTIN(OP_CaseIntGreaterEqual,"case_int_ge");
	INDENT_OUTIN(OP_CaseDefault,		"case_default");
	INDENT_OUTIN(OP_CaseIntEqualAny,	"case_int_eq_any");
	INDENT_OUTIN(OP_CaseIntNotEqualAll, "case_int_ne_all");
	INDENT_OUTIN(OP_CaseBitsSet,		"case_bits_set");
	PASSTHROUGH(OP_EndMultiCase,		"end_multi_case");
	INDENT_OUTIN(OP_CaseIntRange,		"case_int_range");
	PASSTHROUGH(OP_SwitchBreak,			"switch_break");
	case OP_EndSwitch:
		removeIndent();
		removeIndent();
		out = indent + "end_switch";
		PRINT_ARGS;
		break;
	PASSTHROUGH(OP_SetExprIntToExprInt, "setii");
	case OP_SetExprIntToRaw:
		out = indent + formatString("setir %s 0x%x", exprToString(readParm(0)).c_str(), readParm(1));
		break;
	PASSTHROUGH(OP_SetExprFloatToExprFloat,"setff");
	PASSTHROUGH(OP_AddInt,				"addi");
	PASSTHROUGH(OP_SubtractInt,			"subi");
	PASSTHROUGH(OP_MultiplyInt,			"muli");
	PASSTHROUGH(OP_DivideInt,			"divi");
	PASSTHROUGH(OP_ModuloInt,			"modi");
	PASSTHROUGH(OP_AddFloat,			"addf");
	PASSTHROUGH(OP_SubtractFloat,		"subf");
	PASSTHROUGH(OP_MultiplyFloat,		"mulf");
	PASSTHROUGH(OP_DivideFloat,			"divf");
	PASSTHROUGH(OP_MemOpSetBaseInt,		"mo_set_base_int");
	PASSTHROUGH(OP_MemOpReadInt,		"mo_read_int");
	PASSTHROUGH(OP_MemOpReadInt2,		"mo_read_int2");
	PASSTHROUGH(OP_MemOpReadInt3,		"mo_read_int3");
	PASSTHROUGH(OP_MemOpReadInt4,		"mo_read_int4");
	PASSTHROUGH(OP_MemOpReadIntIndexed,	"mo_read_int_indexed");
	PASSTHROUGH(OP_MemOpSetBaseFloat,	"mo_set_base_float");
	PASSTHROUGH(OP_MemOpReadFloat,		"mo_read_float");
	PASSTHROUGH(OP_MemOpReadFloat2,		"mo_read_float2");
	PASSTHROUGH(OP_MemOpReadFloat3,		"mo_read_float3");
	PASSTHROUGH(OP_MemOpReadFloat4,		"mo_read_float4");
	PASSTHROUGH(OP_MemOpReadFloatIndexed,"mo_read_float_indexed");
	PASSTHROUGH(OP_SetUserWordBase,		"set_uw_base");
	PASSTHROUGH(OP_SetUserFlagBase,		"set_uf_base");
	PASSTHROUGH(OP_AllocateUserWordBase,"alloc_uw");
	case OP_AndExpr:
		out = indent + formatString("andi %s %s", exprToString(readParm(0)).c_str(), exprToString(readParm(1), NumericalFormat::Hex).c_str());
		break;
	case OP_AndRaw:
		out = indent + formatString("andr %s 0x%X", exprToString(readParm(0)).c_str(), readParm(1));
		break;
	case OP_OrExpr:
		out = indent + formatString("ori %s %s", exprToString(readParm(0)).c_str(), exprToString(readParm(1), NumericalFormat::Hex).c_str());
		break;
	case OP_OrRaw:
		out = indent + formatString("orr %s 0x%X", exprToString(readParm(0)).c_str(), readParm(1));
		break;
	PASSTHROUGH(OP_ConvertMSToFrames,	"cvt_ms_f");
	PASSTHROUGH(OP_ConvertFramesToMS,	"cvt_f_ms");
	PASSTHROUGH(OP_StoreIntToPtr,		"storei");
	PASSTHROUGH(OP_StoreFloatToPtr,		"storef");
	PASSTHROUGH(OP_LoadIntFromPtr,		"loadi");
	PASSTHROUGH(OP_LoadFloatFromPtr,	"loadf");
	PASSTHROUGH(OP_StoreIntToPtrExpr,	"storei_ind");
	PASSTHROUGH(OP_StoreFloatToPtrExpr,	"storef_ind");
	PASSTHROUGH(OP_LoadIntFromPtrExpr,	"loadi_ind");
	PASSTHROUGH(OP_LoadFloatFromPtrExpr,"loadf_ind");
	PASSTHROUGH(OP_CallCppSync,			"callc");
	PASSTHROUGH(OP_CallScriptAsync,		"callsa");
	PASSTHROUGH(OP_CallScriptAsyncSaveTID,"callsa_tid");
	PASSTHROUGH(OP_CallScriptSync,		"callss");
	PASSTHROUGH(OP_TerminateThread,		"stop_tid");
	PASSTHROUGH(OP_Jump,				"jump");
	PASSTHROUGH(OP_SetThreadPriority,	"set_thread_priority");
	PASSTHROUGH(OP_SetThreadTimeQuantum,"set_thread_quantum");
	PASSTHROUGH(OP_SetThreadTypeMask,	"set_thread_type_mask");
	PASSTHROUGH(OP_ThreadSuspendTypes,	"suspend_types");
	PASSTHROUGH(OP_ThreadResumeTypes,	"resume_types");
	PASSTHROUGH(OP_ThreadSuspendTypesOther,"suspend_types_other");
	PASSTHROUGH(OP_ThreadResumeTypesOther,"resume_types_other");
	PASSTHROUGH(OP_ThreadSuspendTID,	"suspend_tid");
	PASSTHROUGH(OP_ThreadResumeTID,		"resume_tid");
	PASSTHROUGH(OP_CheckThreadRunning,	"check_thread_running");
	INDENT_IN(OP_ThreadStart,			"begin_thread");
	INDENT_IN(OP_ThreadStartSaveTID,	"begin_thread_tid");
	INDENT_OUT(OP_ThreadEnd,			"end_thread");
	INDENT_IN(OP_ThreadChildStart,		"begin_child_thread");
	INDENT_IN(OP_ThreadChildStartSaveTID,"begin_child_thread_tid");
	INDENT_OUT(OP_ThreadChildEnd,		"end_child_thread");
	PASSTHROUGH(OP_DebugOutputString,	"dbg_report");
	// OP_DebugUnk1
	PASSTHROUGH(OP_DebugExprToString,	"dbg_expr_to_string");
	// OP_DebugUnk2
	// OP_DebugUnk3
	default:
		out = indent + formatString("UNK[%02X]", opcode);
		PRINT_ARGS;
		break;
	}

	// Special behavior handling
	switch (opcode)
	{
	case OP_CallScriptAsync:
	case OP_CallScriptAsyncSaveTID:
	case OP_CallScriptSync:
		{
			if (!argCrossRefScripts)
				break;

			uint32_t addr = readLong(address);
			if (categorizeExpr(addr) != ExpressionType::Address)
				break;

			if (!isAddrLoaded(addr))
				break;

			if (std::find(gDisassemblyList.begin(), gDisassemblyList.end(), addr) == gDisassemblyList.end())
			{
				gDisassemblyList.push_back(addr);
			}
		}
		break;
	default:
		break;
	}

	address += param_count * sizeof(uint32_t);

	return out;
}

void disassembleFunction(uint32_t address)
{
	uint32_t addr = address;

	bool done = false;
	std::string indentation = cIndentLevel;
	while (!done)
	{
		printf("%08X: ", addr);
		printf("%s\n", disassembleOpcode(addr, indentation, &done).c_str());
	}
}

int main(int argc, char **argv)
{
	printf("ttydasm v1.0 by PistonMiner, built on " __TIMESTAMP__ "\n\n");

	setupConsoleCodePage();

	{
		// Parse command line args
		namespace po = boost::program_options;

		po::options_description desc("Options");
		desc.add_options()
			("help", "Print help message")
			("start-offset", po::value<std::vector<std::string>>(&argStartOffsetStrings), "Offset to start disassembly from")
			("start-address", po::value<std::vector<std::string>>(&argStartAddressStrings), "Address to start disassembly from")
			("start-symbol", po::value<std::vector<std::string>>(&argStartSymbolStrings), "Symbol to start disassembly from")
			("base-address", po::value<std::string>(&argImageBaseString)->default_value("0x80000000"), "Base address of the input file")
			("symbol-file", po::value<std::vector<std::string>>(&argSymbolFileNames), "Symbol file")
			("crossref-scripts", po::value<bool>(&argCrossRefScripts)->default_value(true), "Automatically disassemble referenced scripts")
			("input-file", po::value<std::string>(&argInputFileName), "Input file");

		po::positional_options_description posOptions;
		posOptions.add("input-file", -1);

		po::store(po::command_line_parser(argc, argv).options(desc).positional(posOptions).run(), gVarMap);
		po::notify(gVarMap);

		if (gVarMap.count("help") || gVarMap.count("input-file") != 1)
		{
			std::cout << desc << "\n";
			return 1;
		}
	}

	// Load input data
	gFileData = static_cast<unsigned char *>(loadFile(argInputFileName, &gFileSize));
	
	for (size_t i = 0; i < argSymbolFileNames.size(); ++i)
	{
		loadSymbolMap(argSymbolFileNames[i]);
	}

	gBaseAddress = strtoul(argImageBaseString.c_str(), nullptr, 16);

	for (auto &startAddress : argStartAddressStrings)
	{
		gDisassemblyList.emplace_back(strtoul(startAddress.c_str(), nullptr, 16));
	}

	for (auto &startOffset : argStartOffsetStrings)
	{
		gDisassemblyList.emplace_back(strtoul(startOffset.c_str(), nullptr, 16) + gBaseAddress);
	}

	for (auto &startSymbol : argStartSymbolStrings)
	{
		bool found = false;

		// Pretty horrific complexity, but hey.
		for (auto &it : gSymbolMap)
		{
			if (it.second == startSymbol)
			{
				gDisassemblyList.emplace_back(it.first);
				found = true;
				break;
			}
		}

		if (!found)
		{
			printf("Symbol [%s] not found\n", startSymbol.c_str());
			return 1;
		}
	}

	// No entry address specified, so we just treat this as a flat file and start at the beginning.
	if (!gDisassemblyList.size())
	{
		gDisassemblyList.push_back(gBaseAddress);
	}

	for (size_t i = 0; i < gDisassemblyList.size(); ++i)
	{ 
		uint32_t nextAddress = gDisassemblyList[i];
		printf("\n--- START OF DISASSEMBLY FOR FUNCTION [%s] AT %08X ---\n", lookupSymbol(nextAddress).c_str(), nextAddress);
		disassembleFunction(nextAddress);
	}

	delete gFileData;

	resetConsoleCodePage();

#ifdef _DEBUG
	system("PAUSE");
#endif
}
