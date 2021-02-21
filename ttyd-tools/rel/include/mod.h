#pragma once

#include "util.h"
#include "console.h"

#include <cstdint>

struct ModInitFunction
{
	ModInitFunction(void (*f)())
	{
		next = sFirst;
		sFirst = this;
		initFunction = f;
	}

	ModInitFunction *next;
	void (*initFunction)();

	static ModInitFunction *sFirst;
};

struct ModUpdateFunction
{
	ModUpdateFunction(void (*f)())
	{
		next = sFirst;
		sFirst = this;
		updateFunction = f;
	}

	ModUpdateFunction *next;
	void (*updateFunction)();

	static ModUpdateFunction *sFirst;
};

#define MOD_INTERNAL_ADD_FUNCTION(type) \
	static void MOD_ANONYMOUS(mod_if_func)(); \
	static type MOD_ANONYMOUS(mod_if_obj)(MOD_ANONYMOUS(mod_if_func)); \
	static void MOD_ANONYMOUS(mod_if_func)()

#define MOD_INIT_FUNCTION() \
	MOD_INTERNAL_ADD_FUNCTION(ModInitFunction)
#define MOD_UPDATE_FUNCTION() \
	MOD_INTERNAL_ADD_FUNCTION(ModUpdateFunction)

namespace mod {

class Mod
{
public:
	Mod();
	void init();

private:
	void updateEarly();
	void draw();

private:
	void (*mPFN_makeKey_trampoline)() = nullptr;

	ConsoleSystem mConsole;
};

}