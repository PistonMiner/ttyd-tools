#include "mod.h"
#include "patch.h"
#include "console.h"

#include <ttyd/seq_logo.h>

#include <cstdio>

namespace mod {

ConIntVar seq_logo_skip("seq_logo_skip", 1);

void (*gTrampoline_seq_logoMain)(ttyd::seqdrv::SeqInfo *);

MOD_INIT_FUNCTION()
{
	// Skip logo
	gTrampoline_seq_logoMain = patch::hookFunction(ttyd::seq_logo::seq_logoMain, [](ttyd::seqdrv::SeqInfo *info)
	{
		if (seq_logo_skip.value > 0)
		{
			// Skip states from H&S fadeout wait directly to demo fadeout start
			// TODO: Skip H&S
#if (TTYD_US || TTYD_EU)
			if (info->state == 8)
			{
				info->state = 17;
			}
#elif TTYD_JP
			// No H&S screen so states are different
			// Skip from first logo fadeout wait directly
			// TODO: More efficient logo skip
			if (info->state == 3)
			{
				info->state = 9;
			}
#endif
		}

		gTrampoline_seq_logoMain(info);
	});
}

ConCommand seq_change_map("seq_change_map", [](const char *text) {
	static char mapName[32];
	static char beroName[32];

	int count = sscanf(text, "%31s %31s", mapName, beroName);
	if (count < 1)
		return;

	if (count < 2)
	{
		beroName[0] = '\0';
	}
	ttyd::seqdrv::seqSetSeq(ttyd::seqdrv::SeqIndex::kMapChange, mapName, beroName);
});

}