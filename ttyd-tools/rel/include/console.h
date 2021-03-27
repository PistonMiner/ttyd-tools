#pragma once

#include "keyboard.h"

#include <gc/types.h>

namespace mod {

class ConCommand
{
public:
	using ExecuteCallback = void (*)(const char *args);

	ConCommand(const char *name, ExecuteCallback executeCb)
	{
		next = sFirst;
		sFirst = this;

		this->name = name;
		this->executeCb = executeCb;
	}

public:
	const char *name;
	ExecuteCallback executeCb;

private:
	ConCommand *next;
	static ConCommand *sFirst;

	friend class ConsoleSystem;
	friend void CC_find(const char *args);
};

class ConIntVar
{
public:
	using ChangedCallback = void (*)(ConIntVar *self, int new_value);

	ConIntVar(const char *name, int value, ChangedCallback changedCb = nullptr)
	{
		next = sFirst;
		sFirst = this;

		this->name = name;
		this->value = value;
		this->changedCb = changedCb;
	}

public:
	const char *name;
	int value;
	ChangedCallback changedCb;

private:
	ConIntVar *next;
	static ConIntVar *sFirst;

	friend class ConsoleSystem;
	friend void CC_find(const char *args);
};


class ConsoleSystem;
extern ConsoleSystem *gConsole;

enum LogLevel
{
	LogLevel_Debug = 0,
	LogLevel_Info,
	LogLevel_Warning,
	LogLevel_Error,
};

class ConsoleSystem
{
public:
	ConsoleSystem()
		: mKeyboard(1)
	{
		gConsole = this;
	}

	void init();
	void update();

	void logInfo(const char *fmt, ...);
	void logWarning(const char *fmt, ...);
	void logError(const char *fmt, ...);
	void logDebug(const char *fmt, ...);

	void logColor(const char *text, gc::color4 color);
	
	void overlay(const char *fmt, ...);

	void setMonospace(bool monospace);
	
private:
	void drawLine(int line, const char *text, gc::color4 color = {0xff,0xff,0xff,0xff});

	void updatePrompt();
	void updateUsbGecko();
	void processCommand(const char *text);
	void disp();

private:
	constexpr static int kMaxColumns = 608 / 8 - 2;
	constexpr static int kNumRowsMonospaceFont = 480 / 8 - 2;
	constexpr static int kNumRowsVariableWidthFont = 440 / 28;
	constexpr static int kMaxRows = kNumRowsMonospaceFont;
	static_assert(kMaxRows >= kNumRowsMonospaceFont);
	static_assert(kMaxRows >= kNumRowsVariableWidthFont);

	bool mIsMonospace;
	int mRowCount;

	char mOverlayBuffer[256] = "";

	struct LogLine
	{
		uint64_t time;
		gc::color4 color;
		char text[kMaxColumns + 1];
	};
	LogLine mLogLines[kMaxRows - 1] = {};

	bool mPromptActive = false;
	char mPromptBuffer[64] = "";
	int mBackspaceHoldTimer = 0;
	Keyboard mKeyboard;

	int mUgBufferSize = 0;
	char mUgBuffer[64];
};

}