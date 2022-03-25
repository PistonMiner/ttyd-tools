#include "mod.h"
#include "util.h"
#include "console.h"

#include <ttyd/animdrv.h>
#include <ttyd/dispdrv.h>

#include <gc/os.h>

#include <cstring>
#include <cstdio>

namespace mod {

using namespace ttyd::animdrv;

static int gApViewPoseId = -1;
static int64_t gApViewLastAnimCycle = 0;

ConIntVar ap_view_tx("ap_view_tx", 0);
ConIntVar ap_view_ty("ap_view_ty", 0);
ConIntVar ap_view_tz("ap_view_tz", 0);
ConIntVar ap_view_scale("ap_view_scale", 20);
ConIntVar ap_view_ry("ap_view_ry", 0);
ConIntVar ap_view_ry_rate("ap_view_ry_rate", 0);

ConIntVar ap_view_anim_id("ap_view_anim_id", 0);
ConIntVar ap_view_anim_cycle("ap_view_anim_cycle", 0);

ConCommand ap_view("ap_view", [](const char *args)
{
	char agb_name[64];
	if (sscanf(args, "%63s", agb_name) != 1)
	{
		agb_name[0] = '\0';
	}

	if (gApViewPoseId != -1)
	{
		animPoseRelease(gApViewPoseId);
		gApViewPoseId = -1;
	}

	if (strlen(agb_name) > 0)
	{
		int poseId = animPoseEntry(agb_name, 0);
		if (poseId < 0)
			poseId = -1;
		gApViewPoseId = poseId;

		// Reset anim ID to avoid going out of bounds on new model
		ap_view_anim_id.value = 0;

		// Reset rotation
		ap_view_ry.value = 0;
	}
});

MOD_UPDATE_FUNCTION()
{
	if (gApViewPoseId == -1)
		return;

	uint8_t *agb_data = (uint8_t *)animPoseGetAnimBaseDataPtr(gApViewPoseId);
	char *agb_name = (char *)(agb_data + 0x004);
	int anim_count = *(int32_t *)(agb_data + 0x148);
	char *anim_name = *(char **)(agb_data + 0x1ac);

	gConsole->overlay("agb: %s\n", agb_name);

	// Cycle animations
	int anim_id = ap_view_anim_id.value;
	int anim_cycle_time_ms = ap_view_anim_cycle.value;
	if (anim_cycle_time_ms)
	{
		int64_t now = gc::os::OSGetTime();
		if (now - gApViewLastAnimCycle >= anim_cycle_time_ms * util::GetTbRate() / 1000)
		{
			gApViewLastAnimCycle = now;

			++anim_id;
			anim_id %= anim_count;
			ap_view_anim_id.value = anim_id;
		}
	}
	if (anim_id < 0 || anim_id >= anim_count)
	{
		gConsole->overlay("anim: invalid\n");
		return;
	}
	anim_name += 0x40 * anim_id;
	gConsole->overlay("anim: %s\n", anim_name);

	// No force reset so duplicate setting is OK
	animPoseSetAnim(gApViewPoseId, anim_name, 0);
	animPoseMain(gApViewPoseId);

	// Rotate
	if (ap_view_ry_rate.value != 0)
	{
		ap_view_ry.value += ap_view_ry_rate.value;
	}
	while (ap_view_ry.value >= 3600)
	{
		ap_view_ry.value -= 3600;
	}
	while (ap_view_ry.value < 0)
	{
		ap_view_ry.value += 3600;
	}

	// FIXME: Transparency doesn't work properly
	ttyd::dispdrv::dispEntry(
		ttyd::dispdrv::CameraId::k3d, 1, 0.f,
		[](ttyd::dispdrv::CameraId camId, void *user)
		{
			gc::mat3x4 mat;
			memset(&mat, 0, sizeof(gc::mat3x4));
			float scale = ap_view_scale.value / 10.f;
			mat.a[0] = scale;
			mat.a[5] = scale;
			mat.a[10] = scale;
			mat.a[3]  = ap_view_tx.value;
			mat.a[7]  = ap_view_ty.value;
			mat.a[11] = ap_view_tz.value;
			float rot_y = ap_view_ry.value * 0.1f;

			animPoseDrawMtx(gApViewPoseId, &mat, 1, rot_y, 1.f);
			animPoseDrawMtx(gApViewPoseId, &mat, 2, rot_y, 1.f);
			animPoseDrawMtx(gApViewPoseId, &mat, 3, rot_y, 1.f);
		},
		nullptr
	);
}

}