#include "mod.h"

#include "patch.h"

#include <ttyd/system.h>
#include <ttyd/mariost.h>
#include <ttyd/fontmgr.h>
#include <ttyd/dispdrv.h>
#include <ttyd/seqdrv.h>
#include <ttyd/seq_logo.h>
#include <ttyd/mario.h>

#include <gc/os.h>

#include <cstdio>
#include <cstring>

namespace mod {

Mod *gMod = nullptr;

void main()
{
	Mod *mod = new Mod();
	mod->init();
}

Mod::Mod()
{
	
}

void Mod::init()
{
	gMod = this;
	
	mPFN_makeKey_trampoline = patch::hookFunction(ttyd::system::makeKey, []()
	{
		gMod->updateEarly();
	});

	// Initialize typesetting early
	ttyd::fontmgr::fontmgrTexSetup();
	patch::hookFunction(ttyd::fontmgr::fontmgrTexSetup, [](){});

	// Keyboard on controller port 1
	mKeyboard = new Keyboard(1);
}

void Mod::updateEarly()
{
	// Keyboard code
	mKeyboard->update();

	// Register draw command
	ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
	{
		reinterpret_cast<Mod *>(user)->draw();
	}, this);
	
	// Keyboard input for prompt
	size_t bufferLen = strlen(mCommandBuffer);
	for (int i = 0; i < mKeyboard->getKeyPressedCount(); ++i)
	{
		KeyCode pressed = mKeyboard->getKeyPressed(i);
		char textInput = Keyboard::getCharForKeycode(
			pressed,
			mKeyboard->isKeyDown(KeyCode::kLeftShift) || mKeyboard->isKeyDown(KeyCode::kRightShift)
		);
		if (textInput != '\0')
		{
			if (bufferLen < sizeof(mCommandBuffer) - 1)
			{
				mCommandBuffer[bufferLen++] = textInput;
				mCommandBuffer[bufferLen] = '\0';
			}
		}
		else if (pressed == KeyCode::kEnter)
		{
			processCommand(mCommandBuffer);
			mCommandBuffer[0] = '\0';
		}
		else if (pressed == KeyCode::kBackspace && bufferLen > 0)
		{
			mCommandBuffer[--bufferLen] = '\0';
			mBackspaceHoldTimer = 0;
		}
	}

	// Backspace repeat handling
	if (mKeyboard->isKeyReleased(KeyCode::kBackspace))
	{
		mBackspaceHoldTimer = 0;
	}
	if (mKeyboard->isKeyDown(KeyCode::kBackspace))
	{
		++mBackspaceHoldTimer;
	}
	if (bufferLen > 0 && (mBackspaceHoldTimer >= 60))
	{
		// Erase one per frame
		mCommandBuffer[--bufferLen] = '\0';
	}

	updateHeapInfo();

	// Call original function
	mPFN_makeKey_trampoline();
}

void Mod::draw()
{
	ttyd::mario::Player *player = ttyd::mario::marioGetPtr();
	char displayBuffer[256];
	sprintf(displayBuffer,
	        "Pos: %.2f %.2f %.2f\r\n"
	        "SpdY: %.2f\r\n"
	        "Cmd: %s\r\n"
	        "%s",
	        player->playerPosition[0], player->playerPosition[1], player->playerPosition[2],
	        player->wJumpVelocityY,
	        mCommandBuffer,
	        mDebugHeapText);
	ttyd::fontmgr::FontDrawStart();
	uint32_t color = 0xFFFFFFFF;
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-272, -40, displayBuffer);
}

void Mod::processCommand(const char *command)
{
	size_t functionNameLength = strchr(command, ' ') - command;
	if (!strncmp(command, "debug_heap", functionNameLength))
	{
		// Read heap ID
		int targetHeap = -1;
		sscanf(command, "debug_heap %d", &targetHeap);
		mDebugHeapId = targetHeap;
	}
	else if (!strncmp(command, "change_map", functionNameLength))
	{
		static char mapName[32];
		static char beroName[32];
		int readArgumentCount = sscanf(command, "change_map %31s %31s", mapName, beroName);
		if (readArgumentCount > 0)
		{
			if (readArgumentCount != 2)
			{
				beroName[0] = '\0';
			}
			ttyd::seqdrv::seqSetSeq(ttyd::seqdrv::SeqIndex::kMapChange, mapName, beroName);
		}
	}
}

void Mod::updateHeapInfo()
{
	if (mDebugHeapId == -1)
	{
		mDebugHeapText[0] = '\0';
		return;
	}

	if (mDebugHeapId >= gc::os::OSAlloc_NumHeaps || mDebugHeapId < 0)
	{
		sprintf(mDebugHeapText, "Heap: %d is not a valid heap\r\n", mDebugHeapId);
		return;
	}

	const gc::os::HeapInfo &heap = gc::os::OSAlloc_HeapArray[mDebugHeapId];

	// Check heap integrity
	bool valid = true;
	gc::os::ChunkInfo *currentChunk = nullptr;
	gc::os::ChunkInfo *prevChunk = nullptr;
	for (currentChunk = heap.firstUsed; currentChunk; currentChunk = currentChunk->next)
	{
		// Check pointer sanity
		auto pointerIsValid = [](void *ptr)
		{
			uint32_t pointerRaw = reinterpret_cast<uint32_t>(ptr);
			return pointerRaw >= 0x80000000 && pointerRaw <= 0x817fffff;
		};
		if (!pointerIsValid(currentChunk))
		{
			valid = false;
			break;
		}

		// Sanity check size
		if (currentChunk->size > 0x17fffff)
		{
			valid = false;
			break;
		}

		// Check linked list integrity
		if (prevChunk != currentChunk->prev)
		{
			valid = false;
			break;
		}

		prevChunk = currentChunk;
	}
	if (!valid)
	{
		sprintf(mDebugHeapText,
			    "Heap: %d corrupt at %08lx\r\n",
			    mDebugHeapId,
			    reinterpret_cast<uint32_t>(currentChunk));
		return;
	}

	// Accumulate used memory
	int usage = 0;
	int chunks = 0;
	for (gc::os::ChunkInfo *chunk = heap.firstUsed; chunk; chunk = chunk->next)
	{
		usage += chunk->size;
		++chunks;
	}

	sprintf(mDebugHeapText,
		    "Heap: id %d, %.2f/%.2fkb, %d cks\r\n",
		    mDebugHeapId,
		    usage / 1024.f,
		    heap.capacity / 1024.f,
		    chunks);
}

}