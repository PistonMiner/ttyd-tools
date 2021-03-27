#include "console.h"

#include "util.h"

#include <ttyd/mario.h>
#include <ttyd/dispdrv.h>
#include <ttyd/fontmgr.h>

#include <gc/demo.h>

#include <cstdarg>
#include <cstring>
#include <cstdio>

namespace mod
{

ConCommand *ConCommand::sFirst = nullptr;
ConIntVar *ConIntVar::sFirst = nullptr;
ConsoleSystem *gConsole = nullptr;

ConIntVar con_show("con_show", 1);
ConIntVar con_mono("con_mono", 1, [](ConIntVar *self, int new_value) {
	self->value = new_value;
	gConsole->setMonospace(new_value ? true : false);
});
ConIntVar con_rainbow("con_rainbow", 0);
ConIntVar con_log_level("con_log_level", 0);
ConIntVar con_log_fade("con_log_fade", 1);
ConIntVar con_log_fade_start("con_log_fade_start", 3000);
ConIntVar con_log_fade_duration("con_log_fade_duration", 1000);

void CC_find(const char *args)
{
	char filter[128] = "";
	if (sscanf(args, "%127s", filter) > 1)
		return;

	for (ConCommand *cc = ConCommand::sFirst; cc; cc = cc->next)
	{
		if (strstr(cc->name, filter))
			gConsole->logInfo("%s [cmd]\n", cc->name);
	}
	for (ConIntVar *cv = ConIntVar::sFirst; cv; cv = cv->next)
	{
		if (strstr(cv->name, filter))
			gConsole->logInfo("%s [int=%d]\n", cv->name, cv->value);
	}
}
ConCommand find("find", CC_find);

static void DemoFontSetColor(gc::color4 color)
{
#if TTYD_US
	// TODO: Add GX TEV definitions
	using PFN_GXSetTevColorIn = void (*)(int stage, int a, int b, int c, int d);
	using PFN_GXSetTevAlphaIn = void (*)(int stage, int a, int b, int c, int d);
	using PFN_GXSetTevColor = void (*)(int stage, gc::color4 *color);
	using PFN_GXSetBlendMode = void (*)(int type, int src_factor, int dst_factor, int logic_op);
	PFN_GXSetTevColorIn GXSetTevColorIn = (PFN_GXSetTevColorIn)0x802b8960;
	PFN_GXSetTevAlphaIn GXSetTevAlphaIn = (PFN_GXSetTevAlphaIn)0x802b89a4;
	PFN_GXSetTevColor GXSetTevColor = (PFN_GXSetTevColor)0x802b8ab8;
	PFN_GXSetBlendMode GXSetBlendMode = (PFN_GXSetBlendMode)0x802b932c;

	// Modify TEV setup
	// a=zero, b=texc, c=c0, d=zero
	GXSetTevColorIn(0, 15, 8, 2, 15);
	// a=zero, b=texa, c=a0, d=zero
	GXSetTevAlphaIn(0, 7, 4, 1, 7);

	// Enable blend
	// dst = src_c * src_a + dst_c * (1 - src_a)
	GXSetBlendMode(1, 4, 5, 0);

	// Set C0
	GXSetTevColor(1, &color);
#endif
}

static int CountLines(const char *text)
{
	int lines = 0;
	for (const char *p = text; *p; ++p)
	{
		if (*p == '\n')
			++lines;
	}
	return lines;
}

void ConsoleSystem::init()
{
	setMonospace(con_mono.value ? true : false);
}

void ConsoleSystem::update()
{
	updatePrompt();

	ttyd::dispdrv::CameraId cam_id;
	if (con_mono.value)
	{
		cam_id = ttyd::dispdrv::CameraId::kDebug;
	}
	else
	{
		cam_id = ttyd::dispdrv::CameraId::kDebug3d;
	}

	// No alpha cutoff
	ttyd::dispdrv::dispEntry(
		cam_id, 0, 0.f,
		[](ttyd::dispdrv::CameraId camId, void *user)
		{
			((ConsoleSystem *)user)->disp();
		},
		this
	);
}

void ConsoleSystem::logInfo(const char *fmt, ...)
{
	if (con_log_level.value > LogLevel_Info)
		return;

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	logColor(buffer, { 0xff, 0xff, 0xff, 0xff });
}

void ConsoleSystem::logWarning(const char *fmt, ...)
{
	if (con_log_level.value > LogLevel_Warning)
		return;

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	logColor(buffer, { 0xff, 0xa0, 0x20, 0xff });
}

void ConsoleSystem::logError(const char *fmt, ...)
{
	if (con_log_level.value > LogLevel_Error)
		return;

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	logColor(buffer, { 0xff, 0x20, 0x20, 0xff });
}

void ConsoleSystem::logDebug(const char *fmt, ...)
{
	if (con_log_level.value > LogLevel_Debug)
		return;

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	logColor(buffer, { 0xff, 0x20, 0xa0, 0xff });
}

void ConsoleSystem::logColor(const char *log_text, gc::color4 color)
{
	// Cut off until it fits
	int num_lines = CountLines(log_text);
	int max_lines = MOD_ARRAYSIZE(mLogLines);
	while (num_lines > max_lines)
	{
		char c = *log_text++;
		if (!c)
			break;
		if (c == '\n')
			--num_lines;
	}
	MOD_ASSERT(num_lines <= max_lines);

	// Scroll log
	int num_scroll_lines = max_lines - num_lines;
	memmove(mLogLines, mLogLines + num_lines, num_scroll_lines * sizeof(LogLine));

	// Update buffer
	int i = max_lines - num_lines;
	const char *start = log_text;
	while (*start)
	{
		// Find end of line
		const char *end = strchr(start, '\n');
		int len;
		if (end)
		{
			len = end - start;
		}
		else
		{
			len = strlen(start);
		}

		// Draw line
		MOD_ASSERT(i >= 0 && i < max_lines);
		LogLine &line = mLogLines[i++];
		line.time = gc::os::OSGetTime();
		line.color = color;

		// Truncate line
		if ((size_t)len > MOD_ARRAYSIZE(line.text) - 1)
			len = MOD_ARRAYSIZE(line.text) - 1;
		memcpy(line.text, start, len);
		line.text[len] = '\0';

		if (!end)
			break;
		start = end + 1;
	}
}

void ConsoleSystem::overlay(const char *fmt, ...)
{
	char buffer[MOD_ARRAYSIZE(mOverlayBuffer)];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	// Trim to fit
	char *text = buffer;
	int buffer_len = strlen(buffer);
	int new_left = MOD_ARRAYSIZE(mOverlayBuffer) - (buffer_len + strlen(mOverlayBuffer) + 1);
	if (new_left < 0)
	{
		text += -new_left;
	}
	strcat(mOverlayBuffer, text);
}

void ConsoleSystem::setMonospace(bool monospace)
{
	if (monospace)
	{
		mRowCount = kNumRowsMonospaceFont;
		mIsMonospace = true;
	}
	else
	{
		mRowCount = kNumRowsVariableWidthFont;
		mIsMonospace = false;
	}
	MOD_ASSERT(mRowCount <= kMaxRows);
}

void ConsoleSystem::drawLine(int line, const char *text, gc::color4 color)
{
	// Skip if zero alpha
	if (!color.a)
		return;

	if (mIsMonospace)
	{
		gc::demo::DEMOInitCaption(0, 608, 480);
		DemoFontSetColor(color);
		gc::demo::DEMOPuts(4, 8 + line * 8, 0, text);
	}
	else
	{
		using namespace ttyd::fontmgr;
		FontDrawStart();
		FontDrawColor((uint8_t *)&color);
		if (con_rainbow.value)
			FontDrawRainbowColor();
		FontDrawEdge();

		// The screenspace area with this is approximately 561.6px by 443.4px
		FontDrawMessage(-271, 210 - 28 * line, text);
	}
}

void ConsoleSystem::updatePrompt()
{
	mKeyboard.update();
	if (mKeyboard.isKeyPressed(KeyCode::kPlus))
	{
		mPromptActive = !mPromptActive;
	}

	if (!mPromptActive)
		return;

	// Keyboard input for prompt
	size_t bufferLen = strlen(mPromptBuffer);
	for (int i = 0; i < mKeyboard.getKeyPressedCount(); ++i)
	{
		KeyCode pressed = mKeyboard.getKeyPressed(i);
		char textInput = Keyboard::getCharForKeycode(
			pressed,
			mKeyboard.isKeyDown(KeyCode::kLeftShift) || mKeyboard.isKeyDown(KeyCode::kRightShift)
		);
		if (textInput != '\0')
		{
			if (bufferLen < sizeof(mPromptBuffer) - 1)
			{
				mPromptBuffer[bufferLen++] = textInput;
				mPromptBuffer[bufferLen] = '\0';
			}
		}
		else if (pressed == KeyCode::kEnter)
		{
			processCommand(mPromptBuffer);
			mPromptBuffer[0] = '\0';
			bufferLen = 0;
		}
		else if (pressed == KeyCode::kBackspace && bufferLen > 0)
		{
			mPromptBuffer[--bufferLen] = '\0';
			mBackspaceHoldTimer = 0;
		}
	}

	// Backspace repeat handling
	if (mKeyboard.isKeyReleased(KeyCode::kBackspace))
	{
		mBackspaceHoldTimer = 0;
	}
	if (mKeyboard.isKeyDown(KeyCode::kBackspace))
	{
		++mBackspaceHoldTimer;
	}
	if (bufferLen > 0 && (mBackspaceHoldTimer >= 40))
	{
		// Erase one per frame
		mPromptBuffer[--bufferLen] = '\0';
	}
}

void ConsoleSystem::processCommand(const char *text)
{
	logInfo("] %s\n", text);

	// Skip leading whitespace
	const char *ident_start = text;
	while (*ident_start == ' ')
		++ident_start;

	// Check for empty line
	if (!*ident_start)
		return;

	// Find end of identifier
	const char *ident_end = strchr(ident_start, ' ');
	int ident_len;
	if (ident_end)
	{
		ident_len = ident_end - ident_start;
	}
	else
	{
		ident_len = strlen(ident_start);
	}

	// Find start of args
	const char *args_text = ident_start + ident_len;
	while (*args_text == ' ')
		++args_text;

	for (ConCommand *cc = ConCommand::sFirst; cc; cc = cc->next)
	{
		if (strlen(cc->name) == (size_t)ident_len && 
		    !strncmp(ident_start, cc->name, ident_len))
		{
			cc->executeCb(args_text);
			return;
		}
	}
	for (ConIntVar *cv = ConIntVar::sFirst; cv; cv = cv->next)
	{
		if (strlen(cv->name) == (size_t)ident_len &&
			!strncmp(ident_start, cv->name, ident_len))
		{
			int new_value;
			if (!*args_text || !sscanf(args_text, "%d", &new_value))
			{
				// Print current value
				logInfo("%s = %d\n", cv->name, cv->value);
				return;
			}

			if (cv->changedCb)
			{
				// Use custom change handler
				cv->changedCb(cv, new_value);
			}
			else
			{
				// Change directly
				cv->value = new_value;
			}
			return;
		}
	}

	// Neither command nor variable
	char ident_text[MOD_ARRAYSIZE(mPromptBuffer)];
	memcpy(ident_text, ident_start, ident_len);
	ident_text[ident_len] = '\0';
	logInfo("Unknown command \"%s\"\n", ident_text);
}

void ConsoleSystem::disp()
{
	if (!con_show.value || (con_show.value == 2 && !mPromptActive))
	{
		// Just clear overlays to avoid stackup
		memset(mOverlayBuffer, 0, sizeof(mOverlayBuffer));
		return;
	}

	// Draw overlays
	int line = 0;
	char *overlay_start = mOverlayBuffer;
	while (*overlay_start && line < mRowCount - 1)
	{
		// Find end of line
		char *overlay_end = strchr(overlay_start, '\n');
		if (overlay_end)
		{
			*overlay_end = '\0';
		}

		// Draw line
		drawLine(line++, overlay_start);

		if (!overlay_end)
			break;
		overlay_start = overlay_end + 1;
	}

	// Clear overlay for next frame
	memset(mOverlayBuffer, 0, sizeof(mOverlayBuffer));

	// Draw log
	int64_t now = gc::os::OSGetTime();
	int num_log_lines = MOD_ARRAYSIZE(mLogLines);
	int num_log_lines_visible = mRowCount - line - 1;
	if (num_log_lines_visible > num_log_lines)
		num_log_lines_visible = num_log_lines;
	int first_visible_log_line = num_log_lines - num_log_lines_visible;
	for (int i = first_visible_log_line; i < num_log_lines; ++i)
	{
		LogLine &log_line = mLogLines[i];

		gc::color4 log_color = log_line.color;
		if (!mPromptActive && con_log_fade.value)
		{
			int64_t elapsed = now - log_line.time;
			int elapsed_ms = (1000 * elapsed) / mod::util::GetTbRate();

			int fade_start_ms = con_log_fade_start.value;
			int fade_duration_ms = con_log_fade_duration.value;
			int fade_end_ms = fade_start_ms + fade_duration_ms;
			uint32_t alpha;
			if (elapsed_ms <= fade_start_ms)
			{
				alpha = log_color.a;
			}
			else if (elapsed_ms >= fade_end_ms)
			{
				alpha = 0;
			}
			else
			{
				float fade_percentage = (fade_end_ms - elapsed_ms) / (float)fade_duration_ms;
				alpha = (int)(log_color.a * fade_percentage);
			}
			log_color.a = alpha;
		}

		drawLine(line++, log_line.text, log_color);
	}
	MOD_ASSERT(line == mRowCount - 1);

	// Draw prompt
	if (mPromptActive)
	{
		char prompt[MOD_ARRAYSIZE(mPromptBuffer) + 3] = "$ ";
		strcat(prompt, mPromptBuffer);

		constexpr static gc::color4 kPromptColor = { 0x32, 0x8B, 0xFF, 0xFF };
		drawLine(mRowCount - 1, prompt, kPromptColor);
	}
}

}