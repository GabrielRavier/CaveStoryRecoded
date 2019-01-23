#include "npc180.h"

#include <array>
#include <cmath>
#include "mathUtils.h"
#include "player.h"
#include "flags.h"
#include "sound.h"
#include "render.h"
#include "bullet.h"
#include "caret.h"
#include "stage.h"
#include "game.h"

using std::array;

void npcAct180(npc * NPC) // Curly, AI
{
	array<RECT, 11> rcLeft;
	array<RECT, 11> rcRight;

	rcLeft[0] = { 0, 96, 16, 112 };
	rcLeft[1] = { 16, 96, 0x20, 112 };
	rcLeft[2] = rcLeft[0];
	rcLeft[3] = { 0x20, 96, 48, 112 };
	rcLeft[4] = rcLeft[0];
	rcLeft[5] = { 48, 96, 64, 112 };
	rcLeft[6] = { 64, 96, 80, 112 };
	rcLeft[7] = rcLeft[5];
	rcLeft[8] = { 80, 96, 96, 112 };
	rcLeft[9] = rcLeft[5];
	rcLeft[10] = { 144, 96, 160, 112 };

	rcRight[0] = { 0, 112, 16, 128 };
	rcRight[1] = { 16, 112, 0x20, 126 };
	rcRight[2] = rcRight[0];
	rcRight[3] = { 0x20, 112, 48, 128 };
	rcRight[4] = rcRight[0];
	rcRight[5] = { 48, 112, 64, 128 };
	rcRight[6] = { 64, 112, 80, 128 };
	rcRight[7] = rcRight[5];
	rcRight[8] = { 80, 112, 96, 128 };
	rcRight[9] = { 48, 112, 64, 128 };
	rcRight[10] = { 144, 112, 160, 128 };

	if (NPC->y >= currentPlayer.y - 0x14000)
	{
		if (gCurlyShootWait)
		{
			NPC->tgt_x = gCurlyShootX;
			NPC->tgt_y = gCurlyShootY;
		}
		else
		{
			NPC->tgt_x = currentPlayer.x;
			NPC->tgt_y = currentPlayer.y;
		}
	}
	else if (NPC->y >= 0x20000)
	{
		NPC->tgt_x = 0;
		NPC->tgt_y = NPC->y;
	}
	else
	{
		NPC->tgt_x = 0x280000;
		NPC->tgt_y = NPC->y;
	}
	if (NPC->xm < 0 && NPC->flag & leftWall)
		NPC->xm = 0;
	if (NPC->xm > 0 && NPC->flag & rightWall)
		NPC->xm = 0;

	enum
	{
		start = 20,
		knockedOut = 40,
		running = 100,
	};

	if (NPC->act_no > 200)
	{
		switch (NPC->act_no)
		{
		case 210:
			NPC->xm -= 0x20;
			NPC->direct = dirLeft;
			if (NPC->flag & ground)
				NPC->act_no = 100;
			break;

		case 300:
			NPC->xm += 0x20;
			NPC->direct = dirRight;
			if (NPC->flag & rightWall)
				++NPC->count1;
			else
				NPC->count1 = 0;
			break;

		case 310:
			NPC->xm += 0x20;
			NPC->direct = dirRight;
			if (NPC->flag & ground)
				NPC->act_no = 100;
			break;
		}
	}
	else if (NPC->act_no == 200)
	{
		NPC->xm -= 0x20;
		NPC->direct = dirLeft;
		if (NPC->flag & leftWall)
			++NPC->count1;
		else
			NPC->count1 = 0;
	}
	else
	{
		switch (NPC->act_no)
		{
		case start:
			NPC->x = currentPlayer.x;
			NPC->y = currentPlayer.y;
			NPC->act_no = running;
			NPC->ani_no = 0;
			createNpc(NPC_CurlyAirTankBubble, 0, 0, 0, 0, dirLeft, NPC);
			if (getFlag(563))
				createNpc(NPC_CurlyAIMachineGun, 0, 0, 0, 0, dirLeft, NPC);
			else
				createNpc(NPC_CurlyAIPolarStar, 0, 0, 0, 0, dirLeft, NPC);
			break;

		case knockedOut:
			NPC->act_no = knockedOut + 1;
			NPC->act_wait = 0;
			NPC->ani_no = 10;
			// Fallthrough
		case knockedOut + 1:
			if (++NPC->act_wait == 750)
			{
				NPC->flag &= ~npc_interact;
				NPC->ani_no = 0;
			}
			if (NPC->act_wait > 1000)
			{
				NPC->act_no = running;
				NPC->ani_no = 0;
				createNpc(NPC_CurlyAirTankBubble, 0, 0, 0, 0, dirLeft, NPC);
				if (getFlag(563))
					createNpc(NPC_CurlyAIMachineGun, 0, 0, 0, 0, dirLeft, NPC);
				else
					createNpc(NPC_CurlyAIPolarStar, 0, 0, 0, 0, dirLeft, NPC);
			}
			break;

		case running:
			NPC->ani_no = 0;
			NPC->xm = 7 * NPC->xm / 8;
			NPC->count1 = 0;
			if (NPC->x <= NPC->tgt_x + 0x2000)
			{
				if (NPC->x < NPC->tgt_x - 0x2000)
				{
					NPC->act_no = 300;
					NPC->ani_no = 1;
					NPC->direct = dirRight;
					NPC->act_wait = random(20, 60);
				}
			}
			else
			{
				NPC->act_no = 200;
				NPC->ani_no = 1;
				NPC->direct = dirLeft;
				NPC->act_wait = random(20, 60);
			}
			break;

		default:
			break;
		}
	}

	if (gCurlyShootWait)
		--gCurlyShootWait;
	if (gCurlyShootWait == 70)
		NPC->count2 = 10;
	if (gCurlyShootWait == 60 && NPC->flag & ground && random(0, 2))
	{
		NPC->count1 = 0;
		NPC->ym = -0x600;
		NPC->ani_no = 1;
		playSound(SFX_QuoteJump);
		if (NPC->x <= NPC->tgt_x)
			NPC->act_no = 310;
		else
			NPC->act_no = 210;
	}

	const auto xDistToTarget = abs(NPC->x - NPC->tgt_x);
	const auto yDistToTarget = NPC->y - NPC->tgt_y;

	if (NPC->act_no == 100)
	{
		if (xDistToTarget + pixelsToUnits(2) >= yDistToTarget)
			NPC->ani_no = 0;
		else
			NPC->ani_no = 5;
	}
	if (NPC->act_no == 210 || NPC->act_no == 310)
	{
		if (xDistToTarget + pixelsToUnits(2) >= yDistToTarget)
			NPC->ani_no = 1;
		else
			NPC->ani_no = 6;
	}
	if (NPC->act_no == 200 || NPC->act_no == 300)
	{
		++NPC->ani_wait;
		if (xDistToTarget + pixelsToUnits(2) >= yDistToTarget)
			NPC->ani_no = NPC->act_wait / 4 % 4 + 1;
		else
			NPC->ani_no = NPC->act_wait / 4 % 4 + 6;

		if (NPC->act_wait)
		{
			--NPC->act_wait;
			if (NPC->flag && NPC->count1 > 10)
			{
				NPC->count1 = 0;
				NPC->ym = -pixelsToUnits(3);
				NPC->act_no += 10;
				NPC->ani_no = 1;
				playSound(SFX_QuoteJump);
			}
		}
		else
		{
			NPC->act_no = 100;
			NPC->ani_no = 0;
		}
	}

	if (NPC->act_no >= 100 && NPC->act_no < 500)
	{
		if (NPC->x >= currentPlayer.x - tilesToUnits(5) && NPC->x <= currentPlayer.x + tilesToUnits(5))
			NPC->ym += 51;
		else if (NPC->flag)
			NPC->ym += 0x10;
		else
			NPC->ym += 51;
	}

	if (NPC->xm > 0x300)
		NPC->xm = 0x300;
	if (NPC->xm < -0x300)
		NPC->xm = -0x300;
	if (NPC->ym > 0x5FF)
		NPC->ym = 0x5FF;

	NPC->x += NPC->xm;
	NPC->y += NPC->ym;

	if (NPC->act_no >= 100 && !(NPC->flag & ground) && NPC->ani_no != 1000)
	{
		if (xDistToTarget + pixelsToUnits(2) >= yDistToTarget)
			NPC->ani_no = 1;
		else
			NPC->ani_no = 6;
	}

	NPC->doRects(rcLeft, rcRight);
}

void curlyNPCCommonStart(npc *NPC)
{
	if (NPC->pNpc->ani_no >= 5)
	{
		if (NPC->pNpc->direct != dirLeft)
		{
			NPC->direct = dirRight;
			NPC->x = NPC->pNpc->x;
		}
		else
		{
			NPC->direct = dirLeft;
			NPC->x = NPC->pNpc->x;
		}
		NPC->y = NPC->pNpc->y - 0x1400;
		NPC->ani_no = 1;
	}
	else
	{
		if (NPC->pNpc->direct != dirLeft)
		{
			NPC->direct = dirRight;
			NPC->x = NPC->pNpc->x + 0x1000;
		}
		else
		{
			NPC->direct = dirLeft;
			NPC->x = NPC->pNpc->x - 0x1000;
		}
		NPC->y = NPC->pNpc->y;
		NPC->ani_no = 0;
	}

	switch (NPC->pNpc->ani_no)
	{
	case 1:
	case 3:
	case 6:
	case 8:
		NPC->y -= 0x200;
		break;

	default:
		break;
	}
}

void curlyNPCCommonBulletSpawn(const npc *NPC, int bulletCode)
{
	if (NPC->ani_no)
	{
		if (NPC->direct != dirLeft)
		{
			createBullet(bulletCode, NPC->x + 0x400, NPC->y - 0x800, dirUp);
			createCaret(NPC->x + 0x400, NPC->y - 0x800, effect_Star);
		}
		else
		{
			createBullet(bulletCode, NPC->x - 0x400, NPC->y - 0x800, dirUp);
			createCaret(NPC->x - 0x400, NPC->y - 0x800, effect_Star);
		}
	}
	else if (NPC->direct != dirLeft)
	{
		createBullet(bulletCode, NPC->x + 0x800, NPC->y + 0x600, dirRight);
		createCaret(NPC->x + 0x800, NPC->y + 0x600, effect_Star);
	}
	else
	{
		createBullet(bulletCode, NPC->x - 0x800, NPC->y + 0x600, dirLeft);
		createCaret(NPC->x - 0x800, NPC->y + 0x600, effect_Star);
	}
}

void npcAct181(npc *NPC) // Curly Machine Gun bullet spawner (projectile)
{
	constexpr array<RECT, 2> rcLeft = { { { 184, 152, 200, 168 }, { 200, 152, 216, 168 } } };
	constexpr array<RECT, 2> rcRight = { { { 184, 168, 200, 184 }, { 200, 168, 216, 184 } } };

	if (NPC->pNpc)
	{
		curlyNPCCommonStart(NPC);
		if (NPC->act_no)
		{
			if (NPC->act_no == 10)
			{
				if (++NPC->act_wait % 12 == 1)
					curlyNPCCommonBulletSpawn(NPC, bullet_PolarStarLevel3);
				if (NPC->act_wait == 60)
					NPC->act_no = 0;
			}
		}
		else if (NPC->pNpc->count2 == 10)
		{
			NPC->pNpc->count2 = 0;
			NPC->act_no = 10;
			NPC->act_wait = 0;
		}

		NPC->doRects(rcLeft, rcRight);
	}
}

void npcAct182(npc * NPC) // Curly Polar Star bullet spawner (projectile)
{
	constexpr array<RECT, 2> rcLeft = { { { 184, 152, 200, 168 }, { 200, 152, 216, 168 } } };
	constexpr array<RECT, 2> rcRight = { { { 184, 168, 200, 184 }, { 200, 168, 216, 184 } } };

	if (NPC->pNpc)
	{
		curlyNPCCommonStart(NPC);

		if (NPC->act_no)
		{
			if (NPC->act_no == 10)
			{
				if (++NPC->act_wait % 6 == 1)
					curlyNPCCommonBulletSpawn(NPC, bullet_MachineGunLevel3);
				if (NPC->act_wait == 60)
					NPC->act_no = 0;
			}
		}
		else if (NPC->pNpc->count2 == 10)
		{
			NPC->pNpc->count2 = 0;
			NPC->act_no = 10;
			NPC->act_wait = 0;
		}

		NPC->doRects(rcLeft, rcRight);
	}
}

void npcAct183(npc * NPC) // Curly Air Bubble
{
	constexpr array<RECT, 2> rcNPC = { { {56, 96, 80, 120}, {80, 96, 104, 120} } };

	if (NPC->pNpc)
	{
		if (!NPC->act_no)
		{
			NPC->x = NPC->pNpc->x;
			NPC->y = NPC->pNpc->y;
			NPC->act_no = 1;
		}
		NPC->x += (NPC->pNpc->x - NPC->x) / 2;
		NPC->y += (NPC->pNpc->y - NPC->y) / 2;

		NPC->animate(1, 0, 1);

		if (NPC->pNpc->flag & water)
			NPC->doRects(rcNPC);
		else
			NPC->rect.right = 0;
	}
}

void npcAct184(npc *NPC) //big moving block in almond
{
	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 1;
		NPC->x += 4096;
		NPC->y += 4096;
		break;
	case 10:
		NPC->act_no = 11;
		NPC->ani_no = 1;
		NPC->act_wait = 0;
		NPC->bits |= npc_ignoreSolid;
		// Fallthrough
	case 11:
		switch (NPC->direct)
		{
		case dirLeft:
			NPC->x -= 128;
			break;
		case dirUp:
			NPC->y -= 128;
			break;
		case dirRight:
			NPC->x += 128;
			break;
		case dirDown:
			NPC->y += 128;
			break;
		default:
			break;
		}

		if (!(++NPC->act_wait & 7))
			playSound(26);
		gViewport.quake = 20;
		break;
	case 20:
		for (int i = 0; i <= 3; ++i)
		{
			createNpc(NPC_Smoke, NPC->x + (random(-12, 12) << 9), NPC->y + 0x2000, 
				random(-341, 341), random(-1536, 0), 0, nullptr, false);
		}
		NPC->act_no = 1;
		break;
	default:
		break;
	}
	if (++NPC->ani_wait > 10)
	{
		NPC->ani_wait = 0;
		++NPC->ani_no;
	}
	if (NPC->ani_no > 3)
		NPC->ani_no = 0;

	NPC->rect.left = (NPC->ani_no - (NPC->ani_no / 3) * 2) * 32;
	NPC->rect.top = 64;
	NPC->rect.right = NPC->rect.left + 32;
	NPC->rect.bottom = NPC->rect.top + 32;
	return;
}

void npcAct185(npc *NPC) //lifting doors thing
{
	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 1;
		NPC->y += 4096;
		break;
	case 10:
		NPC->act_no = 11;
		NPC->ani_no = 1;
		NPC->act_wait = 0;
		NPC->bits |= npc_ignoreSolid;
		// Fallthrough
	case 11:
		switch (NPC->direct)
		{
		case dirLeft:
			NPC->x -= 128;
			break;
		case dirUp:
			NPC->y -= 128;
			break;
		case dirRight:
			NPC->x += 128;
			break;
		case dirDown:
			NPC->y += 128;
			break;
		default:
			break;
		}
		++NPC->act_wait;
		break;
	case 20:
		NPC->y -= 12288;
		NPC->act_no = 1;
		break;
	default:
		break;
	}
	NPC->rect.left = 96;
	NPC->rect.top = 64;
	NPC->rect.right = 112;
	NPC->rect.bottom = 96;
	return;
}

void npcAct186(npc *NPC) //small moving block that goes down
{
	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 1;
		break;
	case 10:
		NPC->act_no = 11;
		NPC->ani_no = 1;
		NPC->act_wait = 0;
		NPC->bits |= npc_ignoreSolid;
		// Fallthrough
	case 11:
		switch (NPC->direct)
		{
		case dirLeft:
			NPC->x -= 128;
			break;
		case dirUp:
			NPC->y -= 128;
			break;
		case dirRight:
			NPC->x += 128;
			break;
		case dirDown:
			NPC->y += 128;
			break;
		default:
			break;
		}
		++NPC->act_wait;
		break;
	case 20:
		break;
	}

	if (++NPC->ani_wait > 10)
	{
		NPC->ani_wait = 0;
		if (++NPC->ani_no > 3)
			NPC->ani_no = 0;
	}

	NPC->rect.left = 48 + ((NPC->ani_no - (NPC->ani_no / 3)*2) * 16);
	NPC->rect.top = 48;
	NPC->rect.right = NPC->rect.left + 16;
	NPC->rect.bottom = NPC->rect.top + 16;
	return;
}

void npcAct187(npc *NPC) //Fuzz
{
	array<RECT, 2> rcLeft;
	rcLeft[0] = { 224, 104, 256, 136 };
	rcLeft[1] = { 256, 104, 288, 136 };
	array<RECT, 2> rcRight;
	rcRight[0] = { 224, 136, 256, 168 };
	rcRight[1] = { 256, 136, 288, 168 };

	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 1;
		NPC->tgt_x = NPC->x;
		NPC->tgt_y = NPC->y;
		NPC->count1 = 120;
		NPC->act_wait = random(0, 50);

		for (int i = 0; i < 5; ++i)
			createNpc(NPC_FuzzEnemy, 0, 0, 0, 0, 51 * i, NPC);
//Fallthrough
	case 1:
		if (++NPC->act_wait >= 50)
		{
			NPC->act_wait = 0;
			NPC->act_no = 2;
			NPC->ym = 0x300;
		}
		break;

	case 2:
		NPC->count1 += 4;

		if (currentPlayer.x >= NPC->x)
			NPC->direct = 2;
		else
			NPC->direct = 0;

		if (NPC->tgt_y < NPC->y)
			NPC->ym -= 0x10;
		if (NPC->tgt_y > NPC->y)
			NPC->ym += 0x10;

		if (NPC->ym > 0x355)
			NPC->ym = 0x355;
		if (NPC->ym < -0x355)
			NPC->ym = -0x355;
		break;

	default:
		break;
	}

	NPC->x += NPC->xm;
	NPC->y += NPC->ym;

	if (++NPC->ani_wait > 2)
	{
		NPC->ani_wait = 0;
		++NPC->ani_no;
	}

	if (NPC->ani_no > 1)
		NPC->ani_no = 0;

	NPC->doRects(rcLeft, rcRight);
}

void npcAct188(npc *NPC) //Baby Fuzz
{
	uint8_t deg;
	array<RECT, 2> rcLeft;
	rcLeft[0] = { 288, 104, 304, 120 };
	rcLeft[1] = { 304, 104, 320, 120 };
	array<RECT, 2> rcRight;
	rcRight[0] = { 288, 120, 304, 136 };
	rcRight[1] = { 304, 120, 320, 136 };

	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 1;
		NPC->count1 = NPC->direct;
//Fallthrough
	case 1:
		if (NPC->pNpc->code_char != 187 || (NPC->pNpc->cond & 0x80u) == 0)
		{
			NPC->xm = random(-0x200, 0x200);
			NPC->ym = random(-0x200, 0x200);
			NPC->act_no = 10;
		}
		else
		{
			deg = (NPC->count1 & 0xFF) + (NPC->pNpc->count1 & 0xFF);

			NPC->x = NPC->pNpc->x + 20 * getSin(deg);
			NPC->y = NPC->pNpc->y + 0x20 * getCos(deg);
		}
		break;

	case 10:
		if (currentPlayer.x >= NPC->x)
			NPC->xm += 0x20;
		else
			NPC->xm -= 0x20;

		if (currentPlayer.y >= NPC->y)
			NPC->ym += 0x20;
		else
			NPC->ym -= 0x20;

		if (NPC->xm > 0x800)
			NPC->xm = 0x800;
		if (NPC->xm < -0x800)
			NPC->xm = -0x800;
		if (NPC->ym > 0x200)
			NPC->ym = 0x200;
		if (NPC->ym < -0x200)
			NPC->ym = -0x200;

		NPC->x += NPC->xm;
		NPC->y += NPC->ym;
		break;

	default:
		break;
	}

	if (currentPlayer.x >= NPC->x)
		NPC->direct = 2;
	else
		NPC->direct = 0;

	if (++NPC->ani_wait > 2)
	{
		NPC->ani_wait = 0;
		++NPC->ani_no;
	}

	if (NPC->ani_no > 1)
		NPC->ani_no = 0;

	NPC->doRects(rcLeft, rcRight);
}

void npcAct190(npc *NPC) //explody robot guy
{
	switch (NPC->act_no)
	{
	case 0:
		NPC->ani_no = 0;
		break;
	case 10:
		playSound(SFX_Explosion);
		for (int i = 0; i <= 7; ++i)
		{
			createNpc(NPC_Smoke, NPC->x, NPC->y + (random(-8, 8) << 9), 
				random(-8, -2) << 9, random(-3, 3) << 9, 0, nullptr, false);
		}
		NPC->cond = 0;
		break;
	case 20:
		if (++NPC->ani_wait > 10)
		{
			NPC->ani_wait = 0;
			++NPC->ani_no;
		}
		if (NPC->ani_no > 1)
			NPC->ani_no = 0;
		break;
	}
	NPC->rect.left = 192+(NPC->ani_no*16);
	NPC->rect.top = 32;
	NPC->rect.right = NPC->rect.left + 16;
	NPC->rect.bottom = NPC->rect.top + 16;
}

void npcAct191(npc *NPC) //water level controller npc
{
	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 10;
		NPC->tgt_y = NPC->y;
		NPC->ym = 512;
		// Fallthrough
	case 10:
		if (NPC->y >= NPC->tgt_y)
			NPC->ym -= 4;
		else
			NPC->ym += 4;

		if (NPC->ym < -256)
			NPC->ym = -256;
		if (NPC->ym > 256)
			NPC->ym = 256;
		NPC->y += NPC->ym;
		break;
	case 20:
		NPC->act_no = 21;
		NPC->act_wait = 0;
		// Fallthrough
	case 21:
		if (NPC->y >= NPC->tgt_y)
			NPC->ym -= 4;
		else
			NPC->ym += 4;

		if (NPC->ym < -512)
			NPC->ym = -512;
		if (NPC->ym > 512)
			NPC->ym = 512;
		NPC->y += NPC->ym;
		if (++NPC->act_wait > 1000)
			NPC->act_no = 22;
		break;
	case 22:
		if (NPC->y >= 0)
			NPC->ym -= 4;
		else
			NPC->ym += 4;
		if (NPC->ym < -512)
			NPC->ym = -512;
		if (NPC->ym > 512)
			NPC->ym = 512;
		NPC->y += NPC->ym;
		if (NPC->y <= 0x7FFF || gSuperYPos)
		{
			NPC->act_no = 21;
			NPC->act_wait = 0;
		}
		break;
	case 30:
		if (NPC->y >= 0)
			NPC->ym -= 4;
		else
			NPC->ym += 4;
		if (NPC->ym < -512)
			NPC->ym = -512;
		if (NPC->ym > 256)
			NPC->ym = 256;
		NPC->y += NPC->ym;
		break;
	default:
		break;
	}
	gWaterY = NPC->y;
	NPC->rect.right = 0;
	NPC->rect.bottom = 0;
	return;
}

void npcAct192(npc *NPC) // Scooter
{
	enum
	{
		parked = 0,
		mounted = 10,
		startEngine = 20,
		takeOff = 30,
		outOfControl = 40,
	};

	switch (NPC->act_no)
	{
	case parked:
		NPC->act_no = parked + 1;
		NPC->view = { tilesToUnits(1), tilesToUnits(0.5), tilesToUnits(1), tilesToUnits(0.5) };
		break;

	case mounted:
		NPC->act_no = mounted + 1;
		NPC->ani_no = 1;
		NPC->view.top = tilesToUnits(1);
		NPC->view.bottom = NPC->view.top;
		NPC->y -= pixelsToUnits(5);
		break;

	case startEngine:
		NPC->act_no = startEngine + 1;
		NPC->act_wait = 1;
		NPC->tgt_x = NPC->x;
		NPC->tgt_y = NPC->y;
		// Fallthrough
	case startEngine + 1:
		NPC->x = NPC->tgt_x + pixelsToUnits(random(-1, 1));
		NPC->y = NPC->tgt_y + pixelsToUnits(random(-1, 1));
		if (++NPC->act_wait > 30)
			NPC->act_no = 30;
		break;

	case takeOff:
		NPC->act_no = takeOff + 1;
		NPC->act_wait = 1;
		NPC->xm = pixelsToUnits(-4);
		NPC->x = NPC->tgt_x;
		NPC->y = NPC->tgt_y;
		playSound(SFX_MissileImpact);
		// Fallthrough
	case takeOff + 1:
		NPC->xm += 0x20;
		NPC->x += NPC->xm;
		++NPC->act_wait;
		NPC->y = NPC->tgt_y + pixelsToUnits(random(-1, 1));
		if (NPC->act_wait > 10)
			NPC->direct = dirRight;
		if (NPC->act_wait > 200)
			NPC->act_no = outOfControl;
		break;

	case outOfControl:
		NPC->act_no = outOfControl + 1;
		NPC->act_wait = 2;
		NPC->direct = dirLeft;
		NPC->y -= tilesToUnits(3);
		NPC->xm = -tilesToUnits(0.5);
		// Fallthrough
	case outOfControl + 1:
		NPC->x += NPC->xm;
		NPC->y += NPC->ym;
		NPC->act_wait += 2;
		if (NPC->act_wait > 1200)
			NPC->cond = 0;
		break;

	default:
		break;
	}
	if (NPC->act_no >= startEngine && !(NPC->act_wait % 4))
	{
		playSound(SFX_FireballBounce);
		if (NPC->direct != dirLeft)
			createCaret(NPC->x - pixelsToUnits(10), NPC->y + pixelsToUnits(10), effect_BoosterSmoke, dirLeft);
		else
			createCaret(NPC->x + pixelsToUnits(10), NPC->y + pixelsToUnits(10), effect_BoosterSmoke, dirRight);
	}

	constexpr array<RECT, 2> rcLeft = { { {224, 64, 256, 80}, {256, 64, 288, 96} } };
	constexpr array<RECT, 2> rcRight = { { {224, 80, 256, 96}, {288, 64, 320, 96} }};

	NPC->doRects(rcLeft, rcRight);
}

void npcAct193(npc *NPC) // Scooter, crashed
{
    if (!NPC->act_no)
    {
        NPC->act_no = 1;
        NPC->x += tilesToUnits(1.5);
    }

    NPC->doRects({256, 96, 320, 112});
}

void npcAct194(npc *NPC) // Blue Robot, destroyed
{
	if (NPC->act_no == 0)
	{
		NPC->act_no = 1;
		NPC->y += 0x800;
	}

	NPC->doRects({ 192, 120, 224, 128 });
}

void npcAct195(npc *NPC) // Grate mouth
{
	NPC->doRects({ 112, 64, 128, 80 });
}

void npcAct196(npc *NPC) //Stream floor
{
	NPC->x -= 0xC00;
	if (NPC->x <= 0x26000)
		NPC->x += 0x2C000;

    NPC->doRects({112, 64, 155, 80}, {112, 80, 144, 96});
}

void npcAct197(npc *NPC) //Porcupine fish
{
	RECT rc[4];

	rc[0].left = 0;
	rc[0].top = 0;
	rc[0].right = 16;
	rc[0].bottom = 16;

	rc[1].left = 16;
	rc[1].top = 0;
	rc[1].right = 32;
	rc[1].bottom = 16;

	rc[2].left = 32;
	rc[2].top = 0;
	rc[2].right = 48;
	rc[2].bottom = 16;

	rc[3].left = 48;
	rc[3].top = 0;
	rc[3].right = 64;
	rc[3].bottom = 16;

	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 10;
		NPC->ani_wait = 0;
		NPC->ym = random(-0x200, 0x200);
		NPC->xm = 0x800;
		// fallthrough
	case 10:
		if (++NPC->ani_wait > 2)
		{
			NPC->ani_wait = 0;
			++NPC->ani_no;
		}

		if (NPC->ani_no > 1)
			NPC->ani_no = 0;

		if (NPC->xm < 0)
		{
			NPC->damage = 3;
			NPC->act_no = 20;
		}

		break;
	case 20:
		NPC->damage = 3;
		if (++NPC->ani_wait > 0)
		{
			NPC->ani_wait = 0;
			++NPC->ani_no;
		}

		if (NPC->ani_no > 3)
			NPC->ani_no = 2;

		if (NPC->x < tilesToUnits(3))
		{
			NPC->destroy_voice = 0;
			killNpc(NPC, 1);
		}

		break;
	}

	if (NPC->flag & 2)
		NPC->ym = 0x200;
	if (NPC->flag & 8)
		NPC->ym = -0x200;

	NPC->xm -= 12;

	NPC->x += NPC->xm;
	NPC->y += NPC->ym;

	NPC->rect.left = rc[NPC->ani_no].left;
	NPC->rect.top = rc[NPC->ani_no].top;
	NPC->rect.right = rc[NPC->ani_no].right;
	NPC->rect.bottom = rc[NPC->ani_no].bottom;
}

void npcAct198(npc *NPC) //Shot (Ironhead)
{
	RECT rcRight[3];

	rcRight[0].left = 208;
	rcRight[0].top = 48;
	rcRight[0].right = 224;
	rcRight[0].bottom = 72;

	rcRight[1].left = 224;
	rcRight[1].top = 48;
	rcRight[1].right = 240;
	rcRight[1].bottom = 72;

	rcRight[2].left = 240;
	rcRight[2].top = 48;
	rcRight[2].right = 256;
	rcRight[2].bottom = 72;

	switch (NPC->act_no)
	{
	case 0:
		if (++NPC->act_wait > 20)
		{
			NPC->act_no = 1;
			NPC->xm = 0;
			NPC->ym = 0;
			NPC->count1 = 0;
		}

		break;
	case 1:
		NPC->xm += 0x20;
		break;
	}

	if (++NPC->ani_wait > 0)
	{
		NPC->ani_wait = 0;
		++NPC->ani_no;
	}

	if (NPC->ani_no > 2)
		NPC->ani_no = 0;

	NPC->x += NPC->xm;
	NPC->y += NPC->ym;

	NPC->rect.left = rcRight[NPC->ani_no].left;
	NPC->rect.top = rcRight[NPC->ani_no].top;
	NPC->rect.right = rcRight[NPC->ani_no].right;
	NPC->rect.bottom = rcRight[NPC->ani_no].bottom;

	if (++NPC->count1 > 100)
		NPC->cond = 0;

	if (NPC->count1 % 4 == 1)
		playSound(46, 1);
}

void npcAct199(npc *NPC) //Current / fan effect
{
	if (!NPC->act_no)
	{
		NPC->act_no = 1;
		NPC->ani_no = random(0, 2);

		const int direction = NPC->direct;

		if (direction == 1)
			NPC->ym = -1;
		else if (direction > 1)
		{
			if (direction == 2)
				NPC->xm = 1;
			else if (direction == 3)
				NPC->ym = 1;
		}
		else if (!direction)
			NPC->xm = -1;

		NPC->xm = (random(4, 8) << 9) / 2 * NPC->xm;
		NPC->ym = (random(4, 8) << 9) / 2 * NPC->ym;
	}

	if (++NPC->ani_wait > 6)
	{
		NPC->ani_wait = 0;
		++NPC->ani_no;
	}

	if (NPC->ani_no > 4)
		NPC->cond = 0;

	NPC->x += NPC->xm;
	NPC->y += NPC->ym;

	NPC->rect = { 72 + (NPC->ani_no << 1),16,74 + (NPC->ani_no << 1),18 };
}

