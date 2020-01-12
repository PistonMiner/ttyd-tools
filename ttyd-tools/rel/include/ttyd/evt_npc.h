#pragma once

#include <cstdint>

#include "evtmgr.h"
#include "npcdrv.h"

namespace ttyd::evt_npc {

struct NpcSetupInfo
{
	const char *name;
	uint32_t initialFlags;
	uint32_t initialReactionFlags;
	const void *initEvtCode;
	const void *regularEvtCode;
	const void *talkEvtCode;
	const void *deadEvtCode;
	const void *findEvtCode;
	const void *lostEvtCode;
	const void *returnEvtCode;
	const void *blowEvtCode;
	npcdrv::NpcTerritoryType territoryType;
	gc::vec3 territoryBase;
	gc::vec3 territorySizeHoming;
	float searchRange;
	float searchAngle;
	float homingRange;
	float homingAngle;
	int32_t battleInfoId;
} __attribute__((__packed__));

static_assert(sizeof(NpcSetupInfo) == 0x5c);

extern "C" {

// evt_npc_entry(char *name, char *modelName)
EVT_DECLARE_USER_FUNC(evt_npc_entry, 2)

// evt_npc_slave_entry(char *name, int slaveIndex, char *modelName, void *deadEvtCode, char *&wOutAnimation)
EVT_DECLARE_USER_FUNC(evt_npc_slave_entry, 5)

// evt_npc_delete(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_delete, 1)

// evt_npc_check_delete(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_check_delete, 1)

// evt_npc_get_ReactionOfLivingBody(int isBattle, int &count)
EVT_DECLARE_USER_FUNC(evt_npc_get_ReactionOfLivingBody, 2)

// evt_npc_setup(const NpcSetupInfo *name)
EVT_DECLARE_USER_FUNC(evt_npc_setup, 1)

// evt_npc_set_position(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_npc_set_position, 4)

// evt_npc_set_width(char *name, float width)
EVT_DECLARE_USER_FUNC(evt_npc_set_width, 2)

// evt_npc_set_height(char *name, float height)
EVT_DECLARE_USER_FUNC(evt_npc_set_height, 2)

// evt_npc_get_height(char *name, float &height)
EVT_DECLARE_USER_FUNC(evt_npc_get_height, 2)

// evt_npc_set_scale(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_npc_set_scale, 4)

// evt_npc_get_scale(char *name, float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_npc_get_scale, 4)

// evt_npc_get_position(char *name, float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_npc_get_position, 4)

// evt_npc_get_home_position(char *name, float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_npc_get_home_position, 4)

// evt_npc_set_home_position(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_npc_set_home_position, 4)

// evt_npc_get_rotate(char *name, float &x, float &y, float &z)
EVT_DECLARE_USER_FUNC(evt_npc_get_rotate, 4)

// evt_npc_set_rotate(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_npc_set_rotate, 4)

// evt_npc_add_rotate(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_npc_add_rotate, 4)

// evt_npc_set_rotate_offset(char *name, float x, float y, float z)
EVT_DECLARE_USER_FUNC(evt_npc_set_rotate_offset, 4)

// evt_npc_move_position(char *name, float x, float z, int timeMs, float wAngle, int flags)
EVT_DECLARE_USER_FUNC(evt_npc_move_position, 6)

// evt_npc_jump_position(char *name, float x, float y, float z, int timeMs, float unk0, float unk1, int unk2)
EVT_DECLARE_USER_FUNC(evt_npc_jump_position, 8)

// evt_npc_jump_position_nohit(char *name, int x, int y, int z, int timeMs, int wHeight)
EVT_DECLARE_USER_FUNC(evt_npc_jump_position_nohit, 6)

// evt_npc_glide_position(char *name, float x, float y, float z, int timeMs, float unk0, float unk1, int wInterpolationMode)
EVT_DECLARE_USER_FUNC(evt_npc_glide_position, 9)

// evt_npc_return_interrupt(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_return_interrupt, 1)

// evt_npc_homing_target(char *name, char *targetName, int timeMs, float unk0, float unk1, float wSpeed, int flags)
EVT_DECLARE_USER_FUNC(evt_npc_homing_target, 7)

// evt_npc_set_ry(char *name, float rotationY)
EVT_DECLARE_USER_FUNC(evt_npc_set_ry, 2)

// evt_npc_set_ry_lr(char *name, float rotationY)
EVT_DECLARE_USER_FUNC(evt_npc_set_ry_lr, 2)

// evt_npc_reverse_ry(char *name, float rotationY)
EVT_DECLARE_USER_FUNC(evt_npc_reverse_ry, 2)

// evt_npc_get_dir(char *name, float &rotationY)
EVT_DECLARE_USER_FUNC(evt_npc_get_dir, 2)

// evt_npc_set_anim(char *name, char *animationName)
EVT_DECLARE_USER_FUNC(evt_npc_set_anim, 2)

// evt_npc_set_damage_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_damage_anim, 1)

// evt_npc_set_confuse_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_confuse_anim, 1)

// evt_npc_set_stop_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_stop_anim, 1)

// evt_npc_set_stay_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_stay_anim, 1)

// evt_npc_set_talk_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_talk_anim, 1)

// evt_npc_set_walk_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_walk_anim, 1)

// evt_npc_set_run_anim(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_set_run_anim, 1)

// evt_npc_set_paper(char *name, char *animGroupName)
EVT_DECLARE_USER_FUNC(evt_npc_set_paper, 2)

// evt_npc_set_paper_anim(char *name, char *animationName)
EVT_DECLARE_USER_FUNC(evt_npc_set_paper_anim, 2)

// evt_npc_clear_paper(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_clear_paper, 1)

// evt_npc_set_force_regl_anim(char *name, char *animationName)
EVT_DECLARE_USER_FUNC(evt_npc_set_force_regl_anim, 2)

// evt_npc_wait_pera(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_wait_pera, 1)

// evt_npc_wait_anim(char *name, float time)
EVT_DECLARE_USER_FUNC(evt_npc_wait_anim, 2)

// evt_npc_flag_onoff(int on, char *name, int mask)
EVT_DECLARE_USER_FUNC(evt_npc_flag_onoff, 3)

// evt_npc_flag_check(char *name, int mask, int &masked)
EVT_DECLARE_USER_FUNC(evt_npc_flag_check, 3)

// evt_npc_reaction_flag_onoff(int on, char *name, int mask)
EVT_DECLARE_USER_FUNC(evt_npc_reaction_flag_onoff, 3)

// evt_npc_status_onoff(int on, char *name, int mask)
EVT_DECLARE_USER_FUNC(evt_npc_status_onoff, 3)

// evt_npc_status_check(char *name, int mask, int &masked)
EVT_DECLARE_USER_FUNC(evt_npc_status_check, 3)

// evt_npc_pera_onoff(char *name, int on)
EVT_DECLARE_USER_FUNC(evt_npc_pera_onoff, 2)

// evt_npc_set_camid(char *name, int camId)
EVT_DECLARE_USER_FUNC(evt_npc_set_camid, 2)

// evt_get_target_dir(char *name, char *otherName, float &direction)
EVT_DECLARE_USER_FUNC(evt_get_target_dir, 3)

// evt_set_dir_to_target(char *name, char *otherName)
EVT_DECLARE_USER_FUNC(evt_set_dir_to_target, 2)

// evt_set_dir_to_target(char *name, char *otherName)
// Rotates the NPC to point towards it's territory base position.
// This function actually parses two parameters and ignores the second one;
// I haven't checked how many are actually used to invoke it in existing evts.
EVT_DECLARE_USER_FUNC(evt_set_dir_to_home, 1)

// evt_npc_add_dirdist(float &inOutX, float &inOutY, float angle, float length)
EVT_DECLARE_USER_FUNC(evt_npc_add_dirdist, 4)

// evt_npc_get_loiter_dir(float &inOutAngle, float offset, float divider)
// Modifies `inOutAngle` to be a random new angle. Used to determine random
// walking directions for NPCs.
// This function seems to be intended as the second argument being a minimum
// offset, and the third argument being a maximum offset. However the
// implementation instead divides a random integer out of the range [0;32767]
// by the distance between the arguments, adds the second argument and then
// maps to 360 degrees.
EVT_DECLARE_USER_FUNC(evt_npc_get_loiter_dir, 3)

// evt_npc_change_interrupt(char *name, int type, raw evtCode)
EVT_DECLARE_USER_FUNC(evt_npc_change_interrupt, 3)

// evt_npc_get_reglid(char *name, int &threadId)
EVT_DECLARE_USER_FUNC(evt_npc_get_reglid, 2)

// evt_npc_restart_regular_event(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_restart_regular_event, 1)

// evt_npc_facedirection_add(char *name, float base, float face)
EVT_DECLARE_USER_FUNC(evt_npc_facedirection_add, 3)

// evt_npc_stop_for_event()
EVT_DECLARE_USER_FUNC(evt_npc_stop_for_event, 0)

// evt_npc_start_for_event()
EVT_DECLARE_USER_FUNC(evt_npc_start_for_event, 0)

// evt_npc_stop_for_one_event(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_stop_for_one_event, 1)

// evt_npc_start_for_one_event(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_start_for_one_event, 1)

// evt_npc_change_fbat_mode(int mode)
EVT_DECLARE_USER_FUNC(evt_npc_change_fbat_mode, 1)

// evt_npc_set_offscreen(char *name, char *offscreenName)
EVT_DECLARE_USER_FUNC(evt_npc_set_offscreen, 2)

// evt_npc_set_color(char *name, int r, int g, int b, int a)
EVT_DECLARE_USER_FUNC(evt_npc_set_color, 5)

// evt_npc_blur_onoff(int on, char *name)
EVT_DECLARE_USER_FUNC(evt_npc_blur_onoff, 2)

// evt_npc_check(char *name, int &exists)
EVT_DECLARE_USER_FUNC(evt_npc_check, 2)

// evt_npc_set_attack_mode(char *name, int attackMode)
EVT_DECLARE_USER_FUNC(evt_npc_set_attack_mode, 2)

// evt_npc_set_battle_info(char *name, int battleInfoId)
EVT_DECLARE_USER_FUNC(evt_npc_set_battle_info, 2)

// evt_npc_set_battle_rule(char *name, int condition, int parameter0, int parameter1)
EVT_DECLARE_USER_FUNC(evt_npc_set_battle_rule, 4)

// evt_npc_battle_start(char *name)
// Starts the active battle attached to an NPC.
// This function actually parses two parameters and ignores the second one;
// Existing code invokes it with one only, we use one for consistency.
EVT_DECLARE_USER_FUNC(evt_npc_battle_start, 1)

// evt_npc_wait_battle_end()
EVT_DECLARE_USER_FUNC(evt_npc_wait_battle_end, 0)

// evt_npc_get_battle_result(int &result)
EVT_DECLARE_USER_FUNC(evt_npc_get_battle_result, 1)

// evt_npc_check_escape_battle(int unk)
EVT_DECLARE_USER_FUNC(evt_npc_check_escape_battle, 1)

// evt_npc_get_battle_rule_keep_result(int &result)
EVT_DECLARE_USER_FUNC(evt_npc_get_battle_rule_keep_result, 1)

// evt_npc_get_btlsetup_work(char *name, int index, int &btlSetupWork)
EVT_DECLARE_USER_FUNC(evt_npc_get_btlsetup_work, 3)

// evt_npc_set_btlsetup_work(char *name, int index, int btlSetupWork)
EVT_DECLARE_USER_FUNC(evt_npc_set_btlsetup_work, 3)

// evt_npc_set_link(char *name, char *otherName)
EVT_DECLARE_USER_FUNC(evt_npc_set_link, 2)

// evt_npc_get_drop_fixitem(char *name, int &fixItem)
EVT_DECLARE_USER_FUNC(evt_npc_get_drop_fixitem, 2)

// evt_npc_get_drop_item(char *name, int &item)
EVT_DECLARE_USER_FUNC(evt_npc_get_drop_item, 2)

// evt_npc_get_drop_heart(char *name, int &hearts)
EVT_DECLARE_USER_FUNC(evt_npc_get_drop_heart, 2)

// evt_npc_get_drop_flower(char *name, int &flowers)
EVT_DECLARE_USER_FUNC(evt_npc_get_drop_flower, 2)

// evt_npc_get_drop_coin(char *name, int &coins)
EVT_DECLARE_USER_FUNC(evt_npc_get_drop_coin, 2)

// evt_npc_getback_item_entry(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_getback_item_entry, 1)

// evt_npc_get_unitwork(char *name, int index, int &unitWork)
EVT_DECLARE_USER_FUNC(evt_npc_get_unitwork, 3)

// evt_npc_set_unitwork(char *name, int index, int unitWork)
EVT_DECLARE_USER_FUNC(evt_npc_set_unitwork, 3)

// evt_npc_wait_msec(char *name, int msec)
EVT_DECLARE_USER_FUNC(evt_npc_wait_msec, 2)

// evt_npc_set_balloontype(char *name, int balloonType)
EVT_DECLARE_USER_FUNC(evt_npc_set_balloontype, 2)

// evt_npc_set_tribe(char *name, char *tribeName)
EVT_DECLARE_USER_FUNC(evt_npc_set_tribe, 2)

// evt_npc_set_autotalkpose(char *name, char *stayPoseName, char *talkPoseName)
EVT_DECLARE_USER_FUNC(evt_npc_set_autotalkpose, 3)

// evt_npc_majo_disp_on(char *name, float x, float y, float z, int unk)
EVT_DECLARE_USER_FUNC(evt_npc_majo_disp_on, 5)

// evt_npc_majo_disp_off(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_majo_disp_off, 1)

// evt_npc_get_kpencount_type(char *name, int &type)
EVT_DECLARE_USER_FUNC(evt_npc_get_kpencount_type, 2)

// evt_npc_calc_score(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_calc_score, 1)

// evt_npc_kamek_move_position(char *name, float wTargetX, float wTargetZ, float unk, int wTimeMsec, float unk)
EVT_DECLARE_USER_FUNC(evt_npc_kamek_move_position, 7)

// evt_npc_kamek_kemuri1(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_kamek_kemuri1, 1)

// evt_npc_kamek_kemuri2(char *name, int wTimeMsec, int unk)
EVT_DECLARE_USER_FUNC(evt_npc_kamek_kemuri2, 3)

// evt_fbat_trans_floor_position(float &x, float &y, float &z, float wOffsetY, float unk)
EVT_DECLARE_USER_FUNC(evt_fbat_trans_floor_position, 5)

// evt_npc_sound_data_reset(char *name)
EVT_DECLARE_USER_FUNC(evt_npc_sound_data_reset, 1)

// evt_npc_sound_data_set(char *name, int moveLeftSfxId, int moveRightSfxId, int unkSfxId, int jumpSfxId, int landingSfxId)
EVT_DECLARE_USER_FUNC(evt_npc_sound_data_set, 5)

// evt_npc_release_filednpc(int on)
EVT_DECLARE_USER_FUNC(evt_npc_release_filednpc, 1)

}

}