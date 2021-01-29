#include <gc/exi.h>

#include "ug.h"

namespace mod
{

bool ugProbe(int chan)
{
	using namespace gc::exi;

	if (!EXIProbe(chan))
		return false;

	// USB Gecko doesn't use normal IDs; we check that the usual ID command
	// just returns zeros to try and avoid sending random commands to e.g.
	// memory cards.
	uint32_t id;
	if (!EXIGetID(chan, 0, &id))
		return false;
	if (id != 0)
		return false;

	if (!EXILock(chan, 0, nullptr))
		return false;
	if (!EXISelect(chan, 0, 5))
	{
		EXIUnlock(chan);
		return false;
	}

	uint16_t cmd = 0x9000;
	if (!EXIImm(chan, &cmd, sizeof(uint16_t), 2, nullptr)
		|| !EXISync(chan))
	{
		EXIDeselect(chan);
		EXIUnlock(chan);
		return false;
	}

	EXIDeselect(chan);
	EXIUnlock(chan);
	return (cmd & 0x0fff) == 0x0470;
}

static int ugTransfer(int chan, void *data, int len, bool write)
{
	using namespace gc::exi;

	// Lock device
	if (!EXILock(chan, 0, nullptr))
	{
		return -1;
	}

	// Set speed
	if (!EXISelect(chan, 0, 5))
	{
		EXIUnlock(chan);
		return -1;
	}

	bool fail = false;

	uint8_t *p = (uint8_t *)data;

	int xfer_len = 0;
	for (; xfer_len < len; ++xfer_len)
	{
		uint16_t cmd;
		if (write)
			cmd = 0xb000 | p[xfer_len] << 4;
		else
			cmd = 0xa000;

		if (!EXIImm(chan, &cmd, sizeof(uint16_t), 2, nullptr) || 
		    !EXISync(chan))
		{
			fail = true;
			break;
		}

		// Exit early if buffers are full
		uint16_t success_mask = write ? 0x0400 : 0x0800;
		if (!(cmd & success_mask))
		{
			break;
		}

		// Read out response byte if reading
		if (!write)
			p[xfer_len] = cmd & 0xff;
	}

	EXIDeselect(chan);
	EXIUnlock(chan);

	return fail ? -1 : xfer_len;
}

int ugSend(int chan, const void *data, int len)
{
	return ugTransfer(chan, (void *)data, len, true);
}

int ugRecv(int chan, void *data, int len)
{
	return ugTransfer(chan, data, len, false);
}

}
