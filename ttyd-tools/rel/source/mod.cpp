#include "mod.h"

#include <ttyd/system.h>
#include <ttyd/mariost.h>
#include <ttyd/fontmgr.h>
#include <ttyd/dispdrv.h>
#include <ttyd/seqdrv.h>
#include <ttyd/seq_logo.h>

#include "patch.h"

#include <cstdio>
#include <math.h>

//This code is extremely unoptimal, so feel free to improve it

namespace mod {

Mod *gMod = nullptr;
uint32_t color = 0xFFFFFFFF;
uint32_t mResetCounter = 0;
float NoPointer = 0;

#ifdef TTYD_US
uint32_t r13 = 0x8041CF20;
char *las_25 = (char *) 0x802C0A84;
char *tik_07 = (char *) 0x802C0308;
char *CurrentRoom = (char *) 0x8041E5C8;
#endif
#ifdef TTYD_JP
uint32_t r13 = 0x80417260;
char *las_25 = (char *) 0x802C2804;
char *tik_07 = (char *) 0x802C2088;
char *CurrentRoom = (char *) 0x804183A8;
#endif
#ifdef TTYD_EU
uint32_t r13 = 0x80429760;
char *las_25 = (char *) 0x802CC604;
char *tik_07 = (char *) 0x802CBE88;
char *CurrentRoom = (char *) 0x8042AEE8;
#endif

extern "C" {
uint32_t strcmp(const char *string1, const char *string2);
uint32_t swByteGet(uint32_t GSWOffset);
uint32_t marioGetPtr();
uint32_t marioGetPartyId();
uint32_t partyGetPtr(uint32_t IDResult);
double __ieee754_atan2(float CoordinateX, float CoordinateY);
}

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

	// Skip the logo
	/*patch::hookFunction(ttyd::seq_logo::seq_logoMain, [](ttyd::seqdrv::SeqInfo *)
	{
		ttyd::seqdrv::seqSetSeq(ttyd::seqdrv::SeqIndex::kTitle, nullptr, nullptr);
	});*/
}

void Mod::updateEarly()
{
	//uint32_t *SequencePosition = *reinterpret_cast<uint32_t **>(r13 + -0x6F50) + 93;
	
	if ((strcmp(CurrentRoom,las_25) == 0) && (swByteGet(0) < 390))
	{
		// Check for font load
		ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
		{
			reinterpret_cast<Mod *>(user)->drawPS();
		}, this);
		
		// Palace skip timing code
		if (ttyd::mariost::marioStGetSystemLevel() == 0xF)
		{
		// Stop upon pausing
		mPalaceSkipTimer.stop();
		//mPalaceSkipTimer.setValue(0);
		mPaused = true;
		}
		else if (ttyd::mariost::marioStGetSystemLevel() == 0 && mPaused)
		{
			// Reset and Start when unpausing
			mPalaceSkipTimer.setValue(0);
			mPalaceSkipTimer.start();
			mPaused = false;
		}
		
		if (ttyd::system::keyGetButtonTrg(0) & 0x0400)
		{
			// Stop when pressing X
			mPalaceSkipTimer.stop();
		}
		
		if (ttyd::system::keyGetButton(0) & 0x0800)
		{
			++mResetCounter;
		}
		else
		{
			mResetCounter = 0;
		}
		
		if (mResetCounter > 120)
		{
			mPalaceSkipTimer.stop();
			mPalaceSkipTimer.setValue(0);
			mResetCounter = 0;
		}
		
		mPalaceSkipTimer.tick();
	}
	else if (strcmp(CurrentRoom,tik_07) == 0)
	{
		uint32_t MarioControl = *reinterpret_cast<uint32_t *>(marioGetPtr());
		MarioControl = MarioControl & (1 << 0); //Check if first bit is active
		
		// Check for font load
		ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
		{
			reinterpret_cast<Mod *>(user)->drawYS();
		}, this);
		
		// Yoshi skip timing code
		if (ttyd::seqdrv::seqGetSeq() == 4) //In battle
		{
		// Pause in battle
		mPalaceSkipTimer.stop();
		mPaused = true;
		}
		else if (MarioControl && mPaused)
		{
			// Reset and Start when leaving battle
			mPalaceSkipTimer.setValue(0);
			mPalaceSkipTimer.start();
			mPaused = false;
		}
		
		if (ttyd::system::keyGetButtonTrg(0) & 0x0100)
		{
			// Stop when pressing A
			mPalaceSkipTimer.stop();
		}
		
		if (ttyd::system::keyGetButton(0) & 0x0800)
		{
			++mResetCounter;
		}
		else
		{
			mResetCounter = 0;
		}
		
		if (mResetCounter > 120)
		{
			mPalaceSkipTimer.stop();
			mPalaceSkipTimer.setValue(0);
			mResetCounter = 0;
		}
		
		mPalaceSkipTimer.tick();
	}
	else
	{
		mPalaceSkipTimer.stop();
		mPalaceSkipTimer.setValue(0);
		mResetCounter = 0;
	}
	
	// Check for font load
	ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
	{
		reinterpret_cast<Mod *>(user)->drawButtonsDisplay();
	}, this);
	
	// Call original function
	mPFN_makeKey_trampoline();
}

void Mod::drawPS()
{
	#ifdef TTYD_US
	float *phantomEmberPos = *reinterpret_cast<float **>(r13 + 0x19A0) + 243;
	uint32_t *ItemArrayPointer = *reinterpret_cast<uint32_t **>(0x803DC294);
	#endif
	#ifdef TTYD_JP
	float *phantomEmberPos = *reinterpret_cast<float **>(r13 + 0x1448) + 243;
	uint32_t *ItemArrayPointer = *reinterpret_cast<uint32_t **>(0x803D8714);
	#endif
	#ifdef TTYD_EU
	float *phantomEmberPos = *reinterpret_cast<float **>(r13 + 0x1A80) + 243;
	uint32_t *ItemArrayPointer = *reinterpret_cast<uint32_t **>(0x803E82F4);
	#endif
	
	//float *marioPos = *reinterpret_cast<float **>(r13 + 0x19E0) + 35;
	float *marioPos = (float *) marioGetPtr() + 35;
	uint32_t PartnerPointer = partyGetPtr(marioGetPartyId());
	float *partnerPos = &NoPointer;
	//float *phantomEmberPos = *reinterpret_cast<float **>(r13 + 0x19A0) + 243;
	//uint32_t *ItemArrayPointer = *reinterpret_cast<uint32_t **>(0x803DC294);
	uint32_t *Item = *reinterpret_cast<uint32_t **>(ItemArrayPointer + 1);
	bool FoundItem = false;
	
	if (PartnerPointer != 0)
	{
		partnerPos = (float *) (PartnerPointer + 0x58 + 0x4); //Y Coordinate
	}
	
	for (int i=0; i<32; i++)
	{
		uint32_t tempitem = (uint32_t) Item;
		uint32_t tempoffset;
		uint16_t tempitemaction;
		uint16_t tempbit;
		
		if ((tempitem == 0) || (tempitem == 121) || (tempitem == 123) || (tempitem == 124))
		{
			tempoffset = (i + 1) * 0x26;
			Item = *reinterpret_cast<uint32_t **>(ItemArrayPointer + tempoffset + 1);
		}
		else
		{
			tempoffset = i * 0x26;
			tempitemaction = *reinterpret_cast<uint16_t *>(ItemArrayPointer + tempoffset + 9);
			if (tempitemaction == 7) //Item is already spawned upon entering the room
			{
				tempoffset = (i + 1) * 0x26;
				Item = *reinterpret_cast<uint32_t **>(ItemArrayPointer + tempoffset + 1);
			}
			else
			{
				tempbit = *reinterpret_cast<uint16_t *>(ItemArrayPointer + tempoffset);
				tempbit = tempbit & (1 << 0); //Check if first bit is active
				if (tempbit) //First bit is active
				{
					Item = *reinterpret_cast<uint32_t **>(ItemArrayPointer + tempoffset + 31);
					FoundItem = true;
					break;
				}
				else
				{
					tempoffset = (i + 1) * 0x26;
					Item = *reinterpret_cast<uint32_t **>(ItemArrayPointer + tempoffset + 1);
				}
			}
		}
	}
	
	if (FoundItem == false)
	{
		Item = (uint32_t *) 0;
	}
	
	sprintf(mDisplayBuffer,
		"PST: %lu\r\nItemTimer: %d\r\nPhaEmY: %.2f\r\nParY: %.2f\r\nMarPos: %.2f %.2f %.2f",
		mPalaceSkipTimer.getValue(),
		Item,
		phantomEmberPos[1],
		partnerPos[0],
		marioPos[0], marioPos[1], marioPos[2]);
	
	ttyd::fontmgr::FontDrawStart();
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-252, -5, mDisplayBuffer);
}

float getStickValue(float Stick)
{
	if (Stick > 127)
	{
		Stick -= 256;
	}
	return Stick;
}

void Mod::drawYS()
{
	float *marioPos = (float *) marioGetPtr() + 35;
	float StickX = getStickValue(ttyd::system::keyGetStickX(0));
	float StickY = getStickValue(ttyd::system::keyGetStickY(0));
	
	double stickAngle = (__ieee754_atan2(StickX,StickY)) * (180 / M_PI);
	if (stickAngle < 0)
	{
		stickAngle += 360;
	}
	
	sprintf(mDisplayBuffer,
		"YST: %lu\r\nStickAngle: %.2f\r\nMarPos: %.2f %.2f %.2f",
		mPalaceSkipTimer.getValue(),
		stickAngle,
		marioPos[0], marioPos[1], marioPos[2]);
	
	ttyd::fontmgr::FontDrawStart();
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-252, -63, mDisplayBuffer);
}

void Mod::drawButtonsDisplay()
{
	char ButtonArray[] = {'(', ')', 'v', '^', 'Z', 'R', 'L', ' ', 'A', 'B', 'X', 'Y', 'S'};
	int DrawX = -252;
	
	for (int i=0;i<13;i++)
	{
		if (i != 7) //Skip unused input value
		{
			if (ttyd::system::keyGetButton(0) & (1 << i))
			{
				sprintf(mDisplayBuffer,
					"%c",
					ButtonArray[i]);
				
				ttyd::fontmgr::FontDrawStart();
				ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
				ttyd::fontmgr::FontDrawEdge();
				ttyd::fontmgr::FontDrawMessage(DrawX, -140, mDisplayBuffer);
			}
			
		DrawX += 20;
		}
	}
}
}