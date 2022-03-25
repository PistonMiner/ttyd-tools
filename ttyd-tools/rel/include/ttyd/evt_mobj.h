#pragma once

#include "evtmgr.h"

namespace ttyd::evt_mobj {

extern "C" {

// evt_mobj_entry(char *mobj_name, char *agb_name)
EVT_DECLARE_USER_FUNC(evt_mobj_entry, 2)

// evt_mobj_delete(char *mobj_name)
EVT_DECLARE_USER_FUNC(evt_mobj_delete, 1)

// evt_mobj_check(char *mobj_name, int &exists)
EVT_DECLARE_USER_FUNC(evt_mobj_check, 2)

// evt_mobj_flag_onoff(int on, char *name, raw mask)
EVT_DECLARE_USER_FUNC(evt_mobj_flag_onoff, 3)

// evt_mobj_get_kindname(char *mobj_name, char *&kind_name)
EVT_DECLARE_USER_FUNC(evt_mobj_get_kindname, 2)

// evt_mobj_set_scale(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_mobj_set_scale, 4)

// evt_mobj_get_position(char *name, float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_mobj_get_position, 4)

// evt_mobj_get_x_position(char *name, float &x)
EVT_DECLARE_USER_FUNC(evt_mobj_get_x_position, 2)

// evt_mobj_get_y_position(char *name, float &y)
EVT_DECLARE_USER_FUNC(evt_mobj_get_y_position, 2)

// evt_mobj_get_z_position(char *name, float &z)
EVT_DECLARE_USER_FUNC(evt_mobj_get_z_position, 2)

// evt_mobj_set_position(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_mobj_set_position, 4)

// evt_mobj_set_x_position(char *name, float x)
EVT_DECLARE_USER_FUNC(evt_mobj_set_x_position, 2)

// evt_mobj_set_y_position(char *name, float y)
EVT_DECLARE_USER_FUNC(evt_mobj_set_y_position, 2)

// evt_mobj_set_z_position(char *name, float z)
EVT_DECLARE_USER_FUNC(evt_mobj_set_z_position, 2)

// evt_mobj_exec_cancel(char *name)
EVT_DECLARE_USER_FUNC(evt_mobj_exec_cancel, 1)

// evt_mobj_set_gravity_bound(char *name, float down_acceleration, float restitution)
EVT_DECLARE_USER_FUNC(evt_mobj_set_gravity_bound, 3)

// evt_mobj_set_anim(char *mobj_name, char *anim_name)
EVT_DECLARE_USER_FUNC(evt_mobj_set_anim, 2)

// evt_mobj_wait_animation_end(char *mobj_name)
EVT_DECLARE_USER_FUNC(evt_mobj_wait_animation_end, 1)

// evt_mobj_set_camid(char *name, int cam_id)
EVT_DECLARE_USER_FUNC(evt_mobj_set_camid, 2)

// evt_mobj_hitevt_onoff(char *name, int on)
EVT_DECLARE_USER_FUNC(evt_mobj_hitevt_onoff, 2)

// evt_mobj_hit_onoff(char *name, int on)
EVT_DECLARE_USER_FUNC(evt_mobj_hit_onoff, 2)

// evt_mobj_switch_blue(int type, char *name, float x, float y, float z, void *evt, int &used)
// Creates a single-use floor switch. `type` determines the variant: 0 creates
// a blue switch, 1 creates a black switch, 2 creates a blue background switch.
EVT_DECLARE_USER_FUNC(evt_mobj_switch_blue, 7)

// evt_mobj_floatswitch_blue(int unused, char *name, float x, float y, float z, void *evt, int &used)
// Creates a blue single-use floating switch.
EVT_DECLARE_USER_FUNC(evt_mobj_floatswitch_blue, 7)

// evt_mobj_tornadoswitch_blue(int unused, char *name, float x, float y, float z, void *evt, int &used)
// Creates a blue single-use large floor switch.
EVT_DECLARE_USER_FUNC(evt_mobj_tornadoswitch_blue, 7)

// evt_mobj_switch_red(int type, char *name, float x, float y, float z, void *evt, int &used)
// Creates a multi-use floor switch. `type` determines the variant: 0 creates a
// red switch, 1 creates a white switch. The `used` flag is unused for this
// MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_switch_red, 7)

// evt_mobj_floatswitch_red(int unused, char *name, float x, float y, float z, void *evt, int &used)
// Creates a red multi-use floor switch. The `used` flag is never set by the
// MOBJ and will break functionality if set when created.
EVT_DECLARE_USER_FUNC(evt_mobj_floatswitch_red, 7)

#if TTYD_EU
// evt_mobj_tornadoswitch_red(int unused, char *name, float x, float y, float z, void *evt, int &used)
// Creates a red multi-use large floor switch. The `used` flag is unused for
// this MOBJ. TODO(LinusS): Verify
// Only available on EU due to `tst`.
EVT_DECLARE_USER_FUNC(evt_mobj_tornadoswitch_red, 7)
#endif

// evt_mobj_timerswitch(int type, char *name1, float x1, float y1, float z1, char *name2, float x2, float y2, float z2, int duration, void *evt, int &used)
// Creates a pair of timer switches. Both switches must be pressed
// within `duration` frames of each other to activate them. `type` determines
// the variant: 0 creates single-use blue switches, 1 creates multi-use red
// switches.
EVT_DECLARE_USER_FUNC(evt_mobj_timerswitch, 12)

#if TTYD_EU
// evt_mobj_rideswitch_orange(char *name, float x, float y, float z, void *evt, int &used)
// Only available on EU due to `tst`.
EVT_DECLARE_USER_FUNC(evt_mobj_rideswitch_orange, 6)

// evt_mobj_rideswitch_green(char *name, float x, float y, float z, void *evt, int &used)
// Only available on EU due to `tst`.
EVT_DECLARE_USER_FUNC(evt_mobj_rideswitch_green, 6)
#endif

// evt_mobj_rideswitch_lightblue(char *name, float x, float y, float z, void *start_evt, void *end_evt, int &used)
// Creates a multi-use stand switch. `start_evt` will be executed when the
// switch is pressed. `end_evt` will be executed when the switch is released.
// The `used` flag is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_rideswitch_lightblue, 7)

// evt_mobj_jumpstand_red(int type, char *name, float x, float y, float z, void *evt, int &used)
// Creates a red trampoline. `type` determines the variant: 0 creates a normal
// trampoline, 1 creates a trampoline with the spring hidden. The `used` flag
// is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_jumpstand_red, 7)

// evt_mobj_jumpstand_blue(int type, char *name, float w_speed, float x, float y, float z, void *evt, int &used)
// Creates a blue trampoline. `type` determines the variant: 0 creates a normal
// trampoline, 1 creates a trampoline with the spring hidden. The `used` flag is
// unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_jumpstand_blue, 8)

// evt_mobj_lv_blk(char *name, float x, float y, float z, void *evt, int &used, char *kind_name)
EVT_DECLARE_USER_FUNC(evt_mobj_lv_blk, 7)

// evt_mobj_float_blk(char *name, float x, float y, float z, raw color, raw size, int unused, int &used)
// Creates a movable block. `color` determines the color: 0 creates a green
// block. 1 creates a purple block. 2 creates an orange block. 3 creates a red
// block. `size` determines the size: 0 creates a small block. 1 creates a
// medium block. 2 creates a large block. If the color is red, the size field
// is ignored and a large block is created. The `used` flag is unused for this
// MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_float_blk, 8)

// evt_mobj_switch_float_blk(char *name, float x, float y, float z, char *float_name, raw color, void *evt, int &used)
// Creates a movable block switch. The switch is linked to the movable block
// MOBJ with the name `float_name`. `color` determines the color: 0 creates a
// green switch. 1 creates a purple switch. 2 creates an orange switch.
// 3 creates a red switch. The `used` flag is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_switch_float_blk, 8)

// evt_mobj_lock_unlock(char *name)
// Unlocks the referenced lock.
EVT_DECLARE_USER_FUNC(evt_mobj_lock_unlock, 1)

// evt_mobj_lock(char *name, int item, float x, float y, float z, float dir, void *interact_evt, void *unlock_evt, int &used)
// Creates a lock. `item` is either an item kind ID or a pointer to a list of
// item kind IDs terminated with -1 that are valid keys to unlock the door.
// `interact_evt` runs when interacting with the door. If `evt_mobj_exec_cancel`
// is called from `interact_evt`, there will be no attempt to open the door.
// Otherwise, the inventory will be checked for a valid item as specified in
// `item`. If none is found, a message will be displayed and the door will stay
// locked. Otherwise, the item is removed from the inventory, the unlock
// animation is played, and `unlock_evt` is ran.
EVT_DECLARE_USER_FUNC(evt_mobj_lock, 9)

// evt_mobj_itembox(char *name, float x, float y, float z, int type, void *interact_evt, void *open_evt, int &used)
// Creates a chest. `type` determines the variant: 0 creates a normal chest,
// 1 creates a big chest, 2 creates a grey chest, 3 creates a black chest,
// 4 creates a golden chest. `interact_evt` runs when interacting with the
// chest. If `evt_mobj_exec_cancel` is called from `interact_evt`, the chest
// won't be opened. Otherwise, the open animation is played and `unlock_evt` is
// ran.
EVT_DECLARE_USER_FUNC(evt_mobj_itembox, 8)

// evt_mobj_signboard(char *name, float x, float y, float z, void *evt, int &used)
// Creates a sign. The `used` flag is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_signboard, 6)

#if TTYD_EU
// evt_mobj_arrow(char *name, float x, float y, float z, int &used)
// Creates an arrow. The `used` flag is unused for this MOBJ.
// Only available on EU due to `tst`.
EVT_DECLARE_USER_FUNC(evt_mobj_arrow, 5)
#endif

// evt_mobj_recovery_blk(char *name, int price, float x, float y, float z, int unused, int &used)
// Creates a recovery block. The `used` flag is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_recovery_blk, 7)

// evt_mobj_save_blk(char *name, float x, float y, float z, int &used)
// Creates a save block. `evt` is ran before the main save sequence. The `used`
// flag is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_save_blk, 6)

#if (TTYD_US || TTYD_EU)
// evt_mobj_blk(char *name, float x, float y, float z, raw type, void *evt, int &used)
// Creates an "empty" block. `type` determines the variant: 0 creates a normal
// block, 1 creates a red block. `evt` is ran when the block is hit. The `used`
// flag is unused for this MOBJ.
// Only available on US/EU due to a difference in `eki`.
EVT_DECLARE_USER_FUNC(evt_mobj_blk, 7)
#endif

// evt_mobj_badgeblk(char *name, float x, float y, float z, int item, void *evt, int &used, raw type)
// Creates a question-mark block. `type` determines the variant: 0 creates a
// normal block, 1 creates a red block. If `evt` is set, run it instead of
// creating the specified item.
EVT_DECLARE_USER_FUNC(evt_mobj_badgeblk, 8)

// evt_mobj_powerupblk(char *name, float x, float y, float z, void *evt, int &used, float dir)
// Creates a shine-sprite block. The `evt` parameter is unused for this MOBJ.
EVT_DECLARE_USER_FUNC(evt_mobj_powerupblk, 7)

// evt_mobj_brick(char *name, float x, float y, float z, int item, raw type, void *evt, int &used)
// Creates a brick or hidden block. `type` determines the variant: 0 creates an
// empty brick block. 1 creates a brick normal item block. 2 creates a brick
// red item block. 3 creates a 10-coin brick block. 10 creates a hidden brick
// block. 11 creates a hidden normal item block. 12 creates a hidden red item
// block. 13 creates a hidden 10-coin brick block.
EVT_DECLARE_USER_FUNC(evt_mobj_brick, 8)

// evt_mobj_breaking_floor(char *name, float x, float y, float z, int type, void *evt)
// Creates a breakable floor panel. `type` determines the variant: 0 creates a
// normal panel, 1 creates a black panel. `evt` is executed when the panel is
// broken. 
EVT_DECLARE_USER_FUNC(evt_mobj_breaking_floor, 7)

// evt_mobj_kururing_floor(char *name, float x, float y, float z, char *mapobj_name, int unused, int &used)
// Creates a hidden star piece floor panel. `mapobj_name` is the name of the
// floor map object.
EVT_DECLARE_USER_FUNC(evt_mobj_kururing_floor, 7)

#if TTYD_EU
// evt_mobj_trap_floor(char *name, float x, float y, float z, char *mapobj_name, int unused, int &used)
// Only available on EU due to `tst`.
EVT_DECLARE_USER_FUNC(evt_mobj_trap_floor, 7)

// evt_mobj_breaking_rock(char *name, float x, float y, float z, void *evt, int &used)
// Creates a rock that can be broken by Bobbery.
// Only available on EU due to `tst`.
EVT_DECLARE_USER_FUNC(evt_mobj_breaking_rock, 6)
#endif

// evt_mobj_koopa_ojama_blk
// evt_mobj_koopa_stone_blk
// evt_mobj_koopa_brick
// evt_mobj_koopa_badgeblk
// evt_mobj_koopa_hidden_blk
// evt_mobj_koopa_blk
// evt_mobj_koopa_dokan
// evt_mobj_koopa_sango
// evt_mobj_koopa_fireber_dodai
// evt_mobj_koopa_pole

}

}