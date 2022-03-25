#include "mod.h"
#include "console.h"

#include <ttyd/mario.h>

#include <cstdio>

namespace mod {

ConCommand mario_set_pos("mario_set_pos", [](const char *args) {
	float x, y, z;
	if (sscanf(args, "%f %f %f", &x, &y, &z) != 3)
		return;

	ttyd::mario::Player *mp = ttyd::mario::marioGetPtr();
	mp->playerPosition[0] = x;
	mp->playerPosition[1] = y;
	mp->playerPosition[2] = z;
});

ConIntVar mario_show_pos("mario_show_pos", 0);
MOD_UPDATE_FUNCTION()
{
	if (!mario_show_pos.value)
		return;

	ttyd::mario::Player *player = ttyd::mario::marioGetPtr();
	gConsole->overlay(
		"Pos: %.2f %.2f %.2f\n"
		"SpdY: %.2f\n",
		player->playerPosition[0], player->playerPosition[1], player->playerPosition[2],
		player->wJumpVelocityY
	);
}

}