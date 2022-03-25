#pragma once

#include "evtmgr.h"

namespace ttyd::evt_bero {

enum BeroInfoType
{
	BeroInfoType_None = 0xf00,
	BeroInfoType_Road = 0,
	BeroInfoType_Door = 1,
	BeroInfoType_Pipe = 2,
	BeroInfoType_Gate = 3,

	BeroInfoType_Background = 0x1000,
	BeroInfoType_NoFade = 0x2000,
};

enum BeroInfoDirectionId
{
	BeroInfoDirectionId_North = 0,
	BeroInfoDirectionId_NorthEast = 1,
	BeroInfoDirectionId_East = 2,
	BeroInfoDirectionId_SouthEast = 3,
	BeroInfoDirectionId_South = 4,
	BeroInfoDirectionId_SouthWest = 5,
	BeroInfoDirectionId_West = 6,
	BeroInfoDirectionId_NorthWest = 7,

	BeroInfoDirectionId_Up = 8,
	BeroInfoDirectionId_Down = 9,

	BeroInfoDirectionId_Invalid = 10000,
	BeroInfoDirectionId_Auto = 20000,
};

enum BeroInfoAnimeType
{
	BeroInfoAnimeType_None = 0,
	BeroInfoAnimeType_Animation = 1,
	BeroInfoAnimeType_Rotation = 2,
	BeroInfoAnimeType_Evt = 4,
	BeroInfoAnimeType_RepeatedEvt = 5,
};

struct BeroInfo
{
	const char *hitName;
	uint16_t type;
	uint16_t sfxId;
	// Direction in 45 degree increments, starting at north going clockwise.
	// See `BeroInfoDirectionId`. Use `Invalid` for no direction. Use `Auto`
	// to derive from `hitName` prefix. Use `Up` and `Down` for pipes.
	int32_t directionId;
	// Position to center the walk animation on. Use { 100000, 0, 0 }
	// to set to hit object position.
	int32_t centerPosition[3];
	// Length to push the end position. Use -1 to use default for type.
	int32_t length;
	void *enterEvtCode;
	// Case kind to use for the exit. Use -1 to use default for type.
	int32_t caseType;
	void *exitEvtCode;
	const char *nextMapName;
	const char *nextBeroName;
	uint16_t enterAnimeType;
	uint16_t exitAnimeType;
	void *enterAnimeArg;
	void *exitAnimeArg;
} __attribute__((__packed__));

static_assert(sizeof(BeroInfo) == 0x3c);

extern "C" {

uint32_t bero_get_BeroEXEC();
int32_t bero_get_BeroNUM();
float bero_get_BeroSX();
float bero_get_BeroSY();
float bero_get_BeroSZ();
float bero_get_BeroEX();
float bero_get_BeroEY();
float bero_get_BeroEZ();
BeroInfo *bero_get_ptr();
void bero_clear_Offset();

// Get the index of a bero by name. If passing in an index, return that index.
int32_t bero_id_filter(char *id);

// evt_bero_mapchange(char *map_name, char *bero_name)
EVT_DECLARE_USER_FUNC(evt_bero_mapchange, 2)

// evt_bero_get_entername(char *&bero_name)
EVT_DECLARE_USER_FUNC(evt_bero_get_entername, 1)

// evt_bero_exec_onoff(int off, int mask)
EVT_DECLARE_USER_FUNC(evt_bero_exec_onoff, 2)

// evt_bero_exec_get(int &exec)
EVT_DECLARE_USER_FUNC(evt_bero_exec_get, 1)

// evt_bero_exec_wait(int mask)
EVT_DECLARE_USER_FUNC(evt_bero_exec_wait, 1)

// evt_bero_get_start_position(float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_bero_get_start_position, 3)

// evt_bero_get_end_position(float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_bero_get_end_position, 3)

// evt_bero_get_info_anime(int index, int &enter_type, int &enter_arg, int &exit_type, int &exit_arg)
EVT_DECLARE_USER_FUNC(evt_bero_get_info_anime, 5)

// evt_bero_get_info_length(int index, float &length)
EVT_DECLARE_USER_FUNC(evt_bero_get_info_length, 2)

// evt_bero_get_info_kinddir(int index, int &flags, int &sfx_id, int direction_id)
EVT_DECLARE_USER_FUNC(evt_bero_get_info_kinddir, 4)

// evt_bero_get_info_nextarea(int index, char *&map_name, char *&bero_name)
EVT_DECLARE_USER_FUNC(evt_bero_get_info_nextarea, 3)

// evt_bero_set_now_number(int index)
EVT_DECLARE_USER_FUNC(evt_bero_set_now_number, 1)

// evt_bero_get_now_number(int &index)
EVT_DECLARE_USER_FUNC(evt_bero_get_now_number, 1)

// evt_bero_get_info()
// Set up beros for current map. A pointer to an array of `BeroInfo` structures,
// terminated with an empty entry, must be passed in LW(0).
EVT_DECLARE_USER_FUNC(evt_bero_get_info, 0)

// evt_bero_get_into_info()
// Get information about the bero used to enter the map.
// LW(1) is set to the bero's index.
// LW(2) is set to the number of beros on the map.
// LW(3) is set to the `hitName` field.
// LW(4) is set to the `type` field.
// LW(5) is set to the effective direction ID.
// LW(6) is set to point to the `centerPosition` field.
// LW(7) is set to point to the enter evt code.
// LW(8) is set to 0xffff0000.
EVT_DECLARE_USER_FUNC(evt_bero_get_into_info, 0)

// evt_bero_id_filter(char *&id)
// Convert bero name to index if not already index. Set `id` to index.
EVT_DECLARE_USER_FUNC(evt_bero_id_filter, 1)

// evt_bero_get_info_num(int index)
// Get information about the specified bero.
// LW(3) is set to the `hitName` field.
// LW(4) is set to the `type` field.
// LW(5) is set to the effective direction ID.
// LW(6) is set to point to the `centerPosition` field.
// LW(7) is set to point to the enter evt code.
// LW(8) is set to 0xffff0000.
// LW(9) is set to the switch status.
// LW(10) is set to the case type.
// LW(11) is set to point to the exit evt code.
// LW(12) is set to 0xffff0000.
// LW(13) is set to the next map name.
// LW(14) is set to the next bero name.
EVT_DECLARE_USER_FUNC(evt_bero_get_info_num, 1)

// bero_get_position_normal(int index, float length)
// Used internally to the bero system. Set up bero start and end position as
// specified by the specified bero.
EVT_DECLARE_USER_FUNC(bero_get_position_normal, 2)

// evt_bero_switch_on(char *id)
// Used internally to the bero system. Mark bero as enabled to prevent
// duplicate enables.
EVT_DECLARE_USER_FUNC(evt_bero_switch_on, 1)

// evt_bero_switch_off(char *id)
// Used internally to the bero system. Mark bero as disabled to prevent
// duplicate disables.
EVT_DECLARE_USER_FUNC(evt_bero_switch_off, 1)

// evt_bero_get_number(char *id)
// Convert bero name to index if not already index. Set LW(1) to index.
EVT_DECLARE_USER_FUNC(evt_bero_get_number, 1)

// evt_bero_case_id_save(int index, int case_id)
// Used internally to the bero system. Store the activation case ID for the
// specified bero.
EVT_DECLARE_USER_FUNC(evt_bero_case_id_save, 2)

// evt_bero_case_id_load(int index, int &case_id)
// Used internally to the bero system. Retrieve the activation case ID for the
// specified bero.
EVT_DECLARE_USER_FUNC(evt_bero_case_id_load, 2)

// evt_bero_overwrite(char *id, BeroInfo *info)
EVT_DECLARE_USER_FUNC(evt_bero_overwrite, 2)

// evt_bero_mario_go_init()
// Used internally to the bero system. Reset the door go flag.
EVT_DECLARE_USER_FUNC(evt_bero_mario_go_init, 0)

// evt_bero_mario_go()
// Used internally to the bero system. Release the door go flag.
EVT_DECLARE_USER_FUNC(evt_bero_mario_go, 0)

// evt_bero_mario_go_wait()
// Used internally to the bero system. Wait for the door go flag to be
// released.
EVT_DECLARE_USER_FUNC(evt_bero_mario_go_wait, 0)

// EVT_DECLARE(evt_bero_mario_enter)
EVT_DECLARE(evt_bero_info_run)
// EVT_DECLARE(evt_bero_info_run_gate)
// EVT_DECLARE(evt_bero_info_run_pipe)
// EVT_DECLARE(evt_bero_info_run_door)
// EVT_DECLARE(evt_bero_info_run_road)
// EVT_DECLARE(evt_bero_info_run_none)
EVT_DECLARE(bero_case_switch_off)
EVT_DECLARE(bero_case_switch_on)
EVT_DECLARE(bero_case_entry)
EVT_DECLARE(bero_play_out_anime)
EVT_DECLARE(bero_play_enter_anime)
EVT_DECLARE(bero_close_door_play_se)
EVT_DECLARE(bero_open_door_play_se)

}

}