#include "mod.h"
#include "console.h"
#include "patch.h"

#include <ttyd/dispdrv.h>
#include <ttyd/evtmgr.h>
#include <gc/demo.h>

namespace mod {

ConIntVar evt_show_table("evt_show_table", 0);

MOD_UPDATE_FUNCTION()
{
	if (!evt_show_table.value)
		return;

	ttyd::dispdrv::dispEntry(ttyd::dispdrv::CameraId::kDebug, 1, 0.f, [](ttyd::dispdrv::CameraId layerId, void *user)
	{
		auto wp = ttyd::evtmgr::evtGetWork();

		if (!wp || !wp->entries)
		{
			return;
		}

		// Draw evt table.
		gc::demo::DEMOInitCaption(2, 608, 480);

		int allocatedCount = 0;
		const int kEvtCapacity = 256;
		const int kColumnCount = 8;
		const int kRowCount = kEvtCapacity / kColumnCount;
		for (int col = 0; col < kColumnCount; ++col)
		{
			for (int row = 0; row < kRowCount; ++row)
			{
				int evtIndex = col * kRowCount + row;
				auto evt = &wp->entries[evtIndex];

				int x = 2 + col * (9 * 8 + 4);
				int y = 64 + row * 8;

				char statusCode;
				if (!(evt->flags & 1))
				{
					statusCode = 'U';
				}
				else if (evt->flags & 0x02)
				{
					statusCode = 'S';
				}
				else if (evt->flags & 0x10)
				{
					statusCode = 'W';
				}
				else
				{
					statusCode = 'R';
				}

				if (evt->flags & 1)
				{
					++allocatedCount;
				}

				void *addr = evt->wCurrentCommandPtr;
				if (!(reinterpret_cast<uint32_t>(addr) & 0x01000000))
				{
					statusCode += 0x20;
				}

				gc::demo::DEMOPrintf(
					x, y, 0,
					"%02x%c%06x",
					evtIndex,
					statusCode,
					reinterpret_cast<uint32_t>(addr) & 0x00ffffff
				);
			}
		}

		gc::demo::DEMOPrintf(2, 56, 0, "capacity: %d/%d", allocatedCount, wp->entryCount);
	}, nullptr);
}

#if TTYD_US
MOD_INIT_FUNCTION()
{
	patch::hookInstruction(
		reinterpret_cast<uint8_t *>(ttyd::evtmgr::evtEntry) + 0x6c,
		[](gc::os::OSContext *context, void *user)
		{
			if (context->ctr == 0)
			{
				gConsole->logError("evt overflow\n");
			}
		}
	);
}
#endif

}