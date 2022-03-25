#include "mod.h"
#include "console.h"

#include <ttyd/event.h>
#include <ttyd/swdrv.h>
#include <ttyd/mario.h>
#include <ttyd/mario_motion.h>
#include <ttyd/mario_party.h>
#include <ttyd/seqdrv.h>

#include <cstring>

namespace mod {

ConCommand event_load("event_load", [](const char *args_text)
{
	using namespace ttyd::event;

	// Find the referenced event
	bool found = false;
	int targetStageId;
	int targetEventId;
	EventStageEventDescription *targetEventInfo;

	int stageCount = eventStgNum();
	for (int stageId = 0; stageId < stageCount; ++stageId)
	{
		auto *stageInfo = eventStgDtPtr(stageId);

		int eventCount = stageInfo->eventCount;
		for (int eventId = 0; eventId < eventCount; ++eventId)
		{
			auto *eventInfo = &stageInfo->events[eventId];

			// Check both ID text and name if available
			found |= !strcmp(eventInfo->textId, args_text);
#ifndef TTYD_JP
			found |= !strcmp(eventInfo->nameEn, args_text);
#endif
			if (found)
			{
				targetStageId = stageId;
				targetEventId = eventId;
				targetEventInfo = eventInfo;
				break;
			}
		}

		if (found)
			break;
	}

	if (!found)
	{
		gConsole->logError("No event with name/ID '%s'!\n", args_text);
		return;
	}

	// Warp to that event

	// Clear GSWs
	ttyd::swdrv::swInit();

	// Execute init functions and find GSW(0).
	// The GSW(0) in the description is the one we will attain after completing
	// the named event, not the one we should have when attempting to complete
	// it. Because some IDs are unassigned and there can be alternates (e.g.
	// Glitzville 1F vs 2F cutscene), we have to find the last different one
	// before the event we actually want to go to and set that.
	int lastGsw0 = 0;
	for (int stageId = 0; stageId <= targetStageId; ++stageId)
	{
		auto *stageInfo = eventStgDtPtr(stageId);

		int eventCount = stageInfo->eventCount;
		if (stageId == targetStageId)
			eventCount = targetEventId + 1;

		for (int eventId = 0; eventId < eventCount; ++eventId)
		{
			auto *eventInfo = &stageInfo->events[eventId];

			// Need to be careful: There's sometimes alternate events, only one
			// of which will actually happen, e.g. Glitzville 1F vs 2F cutscene
			if (eventInfo->gsw0 != targetEventInfo->gsw0)
				lastGsw0 = eventInfo->gsw0;
			
			if (eventInfo->pfnInit)
				eventInfo->pfnInit();
		}
	}

	// Set GSW(0)
	ttyd::swdrv::swByteSet(0, lastGsw0);

	// Set entry motion
	if (targetEventInfo->entryMotionType == 0)
		ttyd::mario_motion::marioChgStayMotion();
	else if (targetEventInfo->entryMotionType == 1)
		ttyd::mario_motion::marioChgShipMotion();

	// Create correct party
	ttyd::mario_party::marioPartyKill();
	auto *mario = ttyd::mario::marioGetPtr();
	mario->prevPartyId[0] = 0;
	mario->prevPartyId[1] = 0;
	if (targetEventInfo->party0Id)
		ttyd::mario_party::marioPartyEntry(targetEventInfo->party0Id);
	if (targetEventInfo->party1Id)
		ttyd::mario_party::marioPartyEntry(targetEventInfo->party1Id);

	// Load the map
	ttyd::seqdrv::seqSetSeq(
		ttyd::seqdrv::SeqIndex::kMapChange,
		(void *)targetEventInfo->map,
		(void *)targetEventInfo->bero
	);
});

}