#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_map {

extern "C" {

// evt_mapobj_trans(int unused, char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_mapobj_trans, 5)

// evt_mapobj_rotate(int unused, char *name, float x, float y, float z):
EVT_DECLARE_USER_FUNC(evt_mapobj_rotate, 5)

// evt_mapobj_scale(int unused, char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_mapobj_scale, 5)

// evt_map_set_fog(int mode, float start, float end, int r, int g, int b)
EVT_DECLARE_USER_FUNC(evt_map_set_fog, 6)

// evt_map_get_fog(int &mode, float &start, float &end, int &r, int &g, int &b)
EVT_DECLARE_USER_FUNC(evt_map_get_fog, 6)

// evt_map_fog_onoff(int on)
EVT_DECLARE_USER_FUNC(evt_map_fog_onoff, 1)

// evt_map_set_blend(int use_blend2, int r, int g, int b, int a)
EVT_DECLARE_USER_FUNC(evt_map_set_blend, 5)

// evt_map_blend_off(int use_blend2)
EVT_DECLARE_USER_FUNC(evt_map_blend_off, 2)

// evt_map_set_flag(int on, char *name, int flags)
EVT_DECLARE_USER_FUNC(evt_map_set_flag, 3)

// evt_map_set_mobj_flag(int on, char *name, int flags)
EVT_DECLARE_USER_FUNC(evt_map_set_mobj_flag, 3)

// evt_mapobj_color(int use_group, char *name, int r, int g, int b, int a)
EVT_DECLARE_USER_FUNC(evt_mapobj_color, 6)

// evt_map_playanim(char *name, int w_time_mode, int w_clock)
EVT_DECLARE_USER_FUNC(evt_map_playanim, 3)

// evt_map_checkanim(char *name, int &done, int &ms_left)
EVT_DECLARE_USER_FUNC(evt_map_checkanim, 3)

// evt_map_pauseanim(int pause_all, char *name)
EVT_DECLARE_USER_FUNC(evt_map_pauseanim, 2)

// evt_map_replayanim(int replay_all, char *name)
EVT_DECLARE_USER_FUNC(evt_map_replayanim, 2)

// evt_map_set_playrate(char *name, float rate)
EVT_DECLARE_USER_FUNC(evt_map_set_playrate, 2)

// evt_mapobj_flag_onoff(int use_group, int on, char *name, raw mask)
EVT_DECLARE_USER_FUNC(evt_mapobj_flag_onoff, 4)

// evt_mapobj_set_offscreen(int use_group, char *mapobj_name, char *offscreen_name)
EVT_DECLARE_USER_FUNC(evt_mapobj_set_offscreen, 3)

// evt_mapobj_clear_offscreen(int use_group, char *mapobj_name)
EVT_DECLARE_USER_FUNC(evt_mapobj_clear_offscreen, 2)

// evt_mapobj_get_position(char *name, int &x, int &y, int &z)
EVT_DECLARE_USER_FUNC(evt_mapobj_get_position, 4)

// evt_map_set_tevcallback(int index, void (*callback)(MapTevCallbackInfo *))
EVT_DECLARE_USER_FUNC(evt_map_set_tevcallback, 2)

// evt_map_set_flush_onoff(int use_group, int on, char *mapobj_name)
EVT_DECLARE_USER_FUNC(evt_map_set_flush_onoff, 3)

// evt_map_set_flush_color(int use_group, char *mapobj_name, int r, int g, int b, int a)
EVT_DECLARE_USER_FUNC(evt_map_set_flush_color, 6)

// evt_map_get_flush_color(char *mapobj_name, int &r, int &g, int &b, int &a)
EVT_DECLARE_USER_FUNC(evt_map_get_flush_color, 5)

// check(int &is_riding)
// Checks if the player is riding the mapobj with the name passed in LW(1).
EVT_DECLARE_USER_FUNC(check, 1)

// check2(int &is_not_riding)
// Checks if the player is not riding the mapobj with the nammed passed in LW(1).
EVT_DECLARE_USER_FUNC(check2, 1)

// evt_map_entry_airport_harbor(int mode, char *mapobj_name, int w_unknown)
EVT_DECLARE_USER_FUNC(evt_map_entry_airport_harbor, 3)

// evt_map_replace_mapobj(char *mapobj_name, int mode)
EVT_DECLARE_USER_FUNC(evt_map_replace_mapobj, 2)

}

}