#include "npcCollision.h"
#include "weapons.h"
#include "caret.h"
#include "script.h"
#include "sound.h"
#include "player.h"
#include "input.h"
#include "boss.h"
#include "game.h"

void putLittleStar(const RECT *rcHit, const player *me)
{
	if (!(me->cond & player_removed) && me->ym < -0x200)
	{
		playSound(SFX_QuoteBumpHead);
		createCaret(me->x, me->y - rcHit->top, effect_HeadbumpSparks);
		createCaret(me->x, me->y - rcHit->top, effect_HeadbumpSparks);
	}
}

//Judging functions
int playerJudgeBlock(const RECT *rcHit, player *me, int tx, int ty)
{
	int hit = 0;

	if (me->y - rcHit->top < (2 * (2 * ty + 1) - 1) << 11
	        && rcHit->bottom + me->y > (2 * (2 * ty - 1) + 1) << 11
	        && me->x - rcHit->left < (2 * tx + 1) << 12
	        && me->x - rcHit->left > tilesToUnits(tx))
	{
		me->x = ((2 * tx + 1) << 12) + rcHit->left;

		if (me->xm < -0x180)
			me->xm = -0x180;

		if (!isKeyDown(gKeyLeft) && me->xm < 0)
			me->xm = 0;

		hit |= leftWall;
	}

	if (me->y - rcHit->top < (2 * (2 * ty + 1) - 1) << 11
	        && rcHit->bottom + me->y > (2 * (2 * ty - 1) + 1) << 11
	        && rcHit->right + me->x > (2 * tx - 1) << 12
	        && rcHit->right + me->x < tilesToUnits(tx))
	{
		me->x = ((2 * tx - 1) << 12) - rcHit->right;

		if (me->xm > 0x180)
			me->xm = 0x180;

		if (!isKeyDown(gKeyRight) && me->xm > 0)
			me->xm = 0;

		hit |= rightWall;
	}

	if (me->x - rcHit->left < ((2 * tx + 1) << 12) - 0x600
	        && rcHit->right + me->x > ((2 * tx - 1) << 12) + 0x600
	        && me->y - rcHit->top < (2 * ty + 1) << 12
	        && me->y - rcHit->top > tilesToUnits(ty))
	{
		me->y = ((2 * ty + 1) << 12) + rcHit->top;

		putLittleStar(rcHit, me);

		if (me->ym < 0)
			me->ym = 0;

		hit |= ceiling;
	}

	if (me->x - rcHit->left < ((2 * tx + 1) << 12) - 0x600
	        && rcHit->right + me->x > ((2 * tx - 1) << 12) + 0x600
	        && rcHit->bottom + me->y > (2 * ty - 1) << 12
	        && rcHit->bottom + me->y < tilesToUnits(ty))
	{
		me->y = ((2 * ty - 1) << 12) - rcHit->bottom;

		if (me->ym > 0x400)
			playSound(SFX_HitGround);

		if (me->ym > 0)
			me->ym = 0;

		hit |= ground;
	}

	return hit;
}

//Ceiling slopes
int playerJudgeTriangleA(const RECT *rcHit, player *me, int tx, int ty)
{
	int hit = 0;

	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y - rcHit->top < tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 + 0x800
	        && me->y + rcHit->bottom > (2 * ty - 1) << 12)
	{
		if (!(me->cond & player_removed) && me->ym < -0x200)
			putLittleStar(rcHit, me);
		if (me->ym < 0)
			me->ym = 0;

		me->y = tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 + 0x800 + rcHit->top;

		hit = ceiling;
	}

	return hit;
}

int playerJudgeTriangleB(const RECT *rcHit, player *me, int tx, int ty)
{
	int hit = 0;

	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y - rcHit->top < tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 - 0x800
	        && me->y + rcHit->bottom > (2 * ty - 1) << 12)
	{
		if (!(me->cond & player_removed) && me->ym < -0x200)
			putLittleStar(rcHit, me);
		if (me->ym < 0)
			me->ym = 0;

		me->y = tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 - 0x800 + rcHit->top;

		hit = ceiling;
	}

	return hit;
}

int playerJudgeTriangleC(const RECT *rcHit, player *me, int tx, int ty)
{
	int hit = 0;

	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y - rcHit->top < tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 - 0x800
	        && me->y + rcHit->bottom > (2 * ty - 1) << 12)
	{
		if (!(me->cond & player_removed) && me->ym < -0x200)
			putLittleStar(rcHit, me);
		if (me->ym < 0)
			me->ym = 0;

		me->y = tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 - 0x800 + rcHit->top;

		hit = ceiling;
	}

	return hit;
}

int playerJudgeTriangleD(const RECT *rcHit, player *me, int tx, int ty)
{
	int hit = 0;

	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y - rcHit->top < tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 + 0x800
	        && me->y + rcHit->bottom > (2 * ty - 1) << 12)
	{
		if (!(me->cond & player_removed) && me->ym < -0x200)
			putLittleStar(rcHit, me);
		if (me->ym < 0)
			me->ym = 0;

		me->y = tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 + 0x800 + rcHit->top;

		hit = ceiling;
	}

	return hit;
}

//Floor slopes
int playerJudgeTriangleE(const RECT *rcHit, player *me, int tx, int ty)
{
	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y + rcHit->bottom > tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 - 0x800
	        && me->y - rcHit->top < (2 * ty + 1) << 12)
	{
		me->y = tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 - 0x800 - rcHit->bottom;

		if (me->ym > 0x400)
			playSound(SFX_HitGround);

		if (me->ym > 0)
			me->ym = 0;

		return (ground | slopeLeft | slopeE);
	}

	return slopeE;
}

int playerJudgeTriangleF(const RECT *rcHit, player *me, int tx, int ty)
{
	if (me->x < (2 * tx + 1) << 12
	        && me->x > (2 * tx - 1) << 12
	        && me->y + rcHit->bottom > tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 + 0x800
	        && me->y - rcHit->top < (2 * ty + 1) << 12)
	{
		me->y = tilesToUnits(ty) + (-0x2000 * tx + me->x) / 2 + 0x800 - rcHit->bottom;

		if (me->ym > 0x400)
			playSound(SFX_HitGround);

		if (me->ym > 0)
			me->ym = 0;

		return (ground | slopeLeft | slopeF);
	}

	return slopeF;
}

int playerJudgeTriangleG(const RECT *rcHit, player *me, int tx, int ty)
{
	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y + rcHit->bottom > tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 + 0x800
	        && me->y - rcHit->top < (2 * ty + 1) << 12)
	{
		me->y = tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 + 0x800 - rcHit->bottom;

		if (me->ym > 0x400)
			playSound(SFX_HitGround);

		if (me->ym > 0)
			me->ym = 0;

		return (ground | slopeRight | slopeG);
	}

	return slopeG;
}

int playerJudgeTriangleH(const RECT *rcHit, player *me, int tx, int ty)
{
	if (me->x > (2 * tx - 1) << 12
	        && me->x < (2 * tx + 1) << 12
	        && me->y + rcHit->bottom > tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 - 0x800
	        && me->y - rcHit->top < (2 * ty + 1) << 12)
	{
		me->y = tilesToUnits(ty) - (-0x2000 * tx + me->x) / 2 - 0x800 - rcHit->bottom;

		if (me->ym > 0x400)
			playSound(SFX_HitGround);

		if (me->ym > 0)
			me->ym = 0;

		return (ground | slopeRight | slopeH);
	}

	return slopeH;
}

int playerJudgeWater(const RECT *rcHit, const player *me, int tx, int ty) attrPure;

int playerJudgeWater(const RECT *rcHit, const player *me, int tx, int ty)
{
	int hit = 0;

	if (me->x - rcHit->left < ((2 * tx + 1) << 12) - 0x600
	        && me->x + rcHit->right > ((2 * tx - 1) << 12) + 0x600
	        && me->y - rcHit->top < ((2 * ty + 1) << 12) - 0x600
	        && me->y + rcHit->bottom > tilesToUnits(ty))
	{
		hit = water;
	}

	return hit;
}

int playerJudgeSpike(const RECT *rcHit, const player *me, int tx, int ty) attrPure;

int playerJudgeSpike(const RECT * /*rcHit*/, const player *me, int tx, int ty)
{
	if (me->x - 0x800 < (4 * tx + 1) << 11
	        && me->x + 0x800 > (4 * tx - 1) << 11
	        && me->y - 0x800 < tilesToUnits(ty) + 0x600
	        && me->y + 0x800 > tilesToUnits(ty) - 0x600)
	{
		return spike;
	}

	return 0;
}

int playerJudgeWaterSpike(const RECT *rcHit, const player *me, int tx, int ty) attrPure;

int playerJudgeWaterSpike(const RECT * /*rcHit*/, const player *me, int tx, int ty)
{
	if (me->x - 0x800 < (4 * tx + 1) << 11
	        && me->x + 0x800 > (4 * tx - 1) << 11
	        && me->y - 0x800 < tilesToUnits(ty) + 0x600
	        && me->y + 0x800 > tilesToUnits(ty) - 0x600)
	{
		return (spike | water | bloodWater);
	}

	return 0;
}

//Current
attrPure int playerJudgeCurrentLeft(const RECT *rcHit, const player *me, int tx, int ty);

int playerJudgeCurrentLeft(const RECT *rcHit, const player *me, int tx, int ty)
{
	if (me->x - rcHit->left < (4 * (2 * tx + 1) - 1) << 10
	        && me->x + rcHit->right > (4 * (2 * tx - 1) + 1) << 10
	        && me->y - rcHit->top < (4 * (2 * ty + 1) - 1) << 10
	        && me->y + rcHit->bottom > (4 * (2 * ty - 1) + 1) << 10)
	{
		return windLeft;
	}

	return 0;
}

int playerJudgeCurrentUp(const RECT *rcHit, const player *me, int tx, int ty) attrPure;

int playerJudgeCurrentUp(const RECT *rcHit, const player *me, int tx, int ty)
{
	if (me->x - rcHit->left < (4 * (2 * tx + 1) - 1) << 10
	        && me->x + rcHit->right > (4 * (2 * tx - 1) + 1) << 10
	        && me->y - rcHit->top < (4 * (2 * ty + 1) - 1) << 10
	        && me->y + rcHit->bottom > (4 * (2 * ty - 1) + 1) << 10)
	{
		return windUp;
	}

	return 0;
}

int playerJudgeCurrentRight(const RECT *rcHit, const player *me, int tx, int ty) attrPure;

int playerJudgeCurrentRight(const RECT *rcHit, const player *me, int tx, int ty)
{
	if (me->x - rcHit->left < (4 * (2 * tx + 1) - 1) << 10
	        && me->x + rcHit->right > (4 * (2 * tx - 1) + 1) << 10
	        && me->y - rcHit->top < (4 * (2 * ty + 1) - 1) << 10
	        && me->y + rcHit->bottom > (4 * (2 * ty - 1) + 1) << 10)
	{
		return windRight;
	}

	return 0;
}

attrPure int playerJudgeCurrentDown(const RECT *rcHit, const player *me, int tx, int ty);

int playerJudgeCurrentDown(const RECT *rcHit, const player *me, int tx, int ty)
{
	if (me->x - rcHit->left < (4 * (2 * tx + 1) - 1) << 10
	        && me->x + rcHit->right > (4 * (2 * tx - 1) + 1) << 10
	        && me->y - rcHit->top < (4 * (2 * ty + 1) - 1) << 10
	        && me->y + rcHit->bottom > (4 * (2 * ty - 1) + 1) << 10)
	{
		return windDown;
	}

	return 0;
}

//Main function
void playerHitMap()
{
	player *me = &currentPlayer;
	const RECT *rcHit = &me->hit;

	me->flag = 0;

	const int fromX = unitsToTiles(me->x - rcHit->left + 0x1000);
	const int fromY = unitsToTiles(me->y - rcHit->top + 0x1000);
	const int toX = unitsToTiles(me->x + rcHit->right + 0x1000);
	const int toY = unitsToTiles(me->y + rcHit->bottom + 0x1000);

	for (int currentX = fromX; currentX <= toX; currentX++)
	{
		for (int currentY = fromY; currentY <= toY; currentY++)
		{
			switch (getTileAttribute(currentX, currentY))
			{
			//Solid
			case 0x05:
			case 0x41:
			case 0x43:
			case 0x46:
				me->flag |= playerJudgeBlock(rcHit, me, currentX, currentY);
				break;

			//Spike
			case 0x42:
				me->flag |= playerJudgeSpike(rcHit, me, currentX, currentY);
				break;

			//Ceiling slopes
			case 0x50:
				me->flag |= playerJudgeTriangleA(rcHit, me, currentX, currentY);
				break;

			case 0x51:
				me->flag |= playerJudgeTriangleB(rcHit, me, currentX, currentY);
				break;

			case 0x52:
				me->flag |= playerJudgeTriangleC(rcHit, me, currentX, currentY);
				break;

			case 0x53:
				me->flag |= playerJudgeTriangleD(rcHit, me, currentX, currentY);
				break;

			//Floor slopes
			case 0x54:
				me->flag |= playerJudgeTriangleE(rcHit, me, currentX, currentY);
				break;

			case 0x55:
				me->flag |= playerJudgeTriangleF(rcHit, me, currentX, currentY);
				break;

			case 0x56:
				me->flag |= playerJudgeTriangleG(rcHit, me, currentX, currentY);
				break;

			case 0x57:
				me->flag |= playerJudgeTriangleH(rcHit, me, currentX, currentY);
				break;

			//Water, water solid, and water spike
			case 0x60:
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x61:
				me->flag |= playerJudgeBlock(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x62:
				me->flag |= playerJudgeWaterSpike(rcHit, me, currentX, currentY);
				break;

			//Water slopes
			//Ceiling slopes
			case 0x70:
				me->flag |= playerJudgeTriangleA(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x71:
				me->flag |= playerJudgeTriangleB(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x72:
				me->flag |= playerJudgeTriangleC(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x73:
				me->flag |= playerJudgeTriangleD(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			//Floor slopes
			case 0x74:
				me->flag |= playerJudgeTriangleE(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x75:
				me->flag |= playerJudgeTriangleF(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x76:
				me->flag |= playerJudgeTriangleG(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0x77:
				me->flag |= playerJudgeTriangleH(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			//Wind
			case 0x80:
				me->flag |= playerJudgeCurrentLeft(rcHit, me, currentX, currentY);
				break;

			case 0x81:
				me->flag |= playerJudgeCurrentUp(rcHit, me, currentX, currentY);
				break;

			case 0x82:
				me->flag |= playerJudgeCurrentRight(rcHit, me, currentX, currentY);
				break;

			case 0x83:
				me->flag |= playerJudgeCurrentDown(rcHit, me, currentX, currentY);
				break;

			//Water current
			case 0xA0:
				me->flag |= playerJudgeCurrentLeft(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0xA1:
				me->flag |= playerJudgeCurrentUp(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0xA2:
				me->flag |= playerJudgeCurrentRight(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			case 0xA3:
				me->flag |= playerJudgeCurrentDown(rcHit, me, currentX, currentY);
				me->flag |= playerJudgeWater(rcHit, me, currentX, currentY);
				break;

			//Default
			default:
				break;
			}
		}
	}

	if (me->y > gWaterY + 2048)
		me->flag |= water;
}

//Npc hit functions
int playerHitNpcSoftSolid(const RECT *rcHit, player *me, const npc *NPC)
{
	int hit = 0;

	if (me->y - rcHit->top < NPC->y + NPC->hit.bottom - 0x600
	        && me->y + rcHit->bottom > NPC->y - NPC->hit.top + 0x600
	        && me->x - rcHit->left < NPC->x + NPC->hit.right
	        && me->x - rcHit->left > NPC->x)
	{
		if (me->xm <= 0x1FF)
			me->xm += 0x200;

		hit = leftWall;
	}

	if (me->y - rcHit->top < NPC->y + NPC->hit.bottom - 0x600
	        && me->y + rcHit->bottom > NPC->y - NPC->hit.top + 0x600
	        && me->x + rcHit->right - 0x200 > NPC->x - NPC->hit.right
	        && me->x + rcHit->right - 0x200 < NPC->x)
	{
		if (me->xm >= -0x1FF)
			me->xm -= 0x200;

		hit |= rightWall;
	}

	if (me->x - rcHit->left < NPC->x + NPC->hit.right - 0x600
	        && me->x + rcHit->right > NPC->x - NPC->hit.right + 0x600
	        && me->y - rcHit->top < NPC->y + NPC->hit.bottom
	        && me->y - rcHit->top > NPC->y)
	{
		if (me->ym < 0)
			me->ym = 0;

		hit |= ceiling;
	}

	if (me->x - rcHit->left < NPC->x + NPC->hit.right - 0x600
	        && me->x + rcHit->right > NPC->x - NPC->hit.right + 0x600
	        && me->y + rcHit->bottom > NPC->y - NPC->hit.top
	        && me->y + rcHit->bottom < NPC->y + 0x600)
	{
		if (NPC->bits & npc_bouncy)
		{
			me->ym = NPC->ym - 0x200;

			hit |= ground;
		}
		else if (!(me->flag & ground) && me->ym > NPC->ym)
		{
			me->y = NPC->y - NPC->hit.top - rcHit->bottom + 0x200;
			me->ym = NPC->ym;

			me->x += NPC->xm;
			hit |= ground;
		}
	}

	return hit;
}

int playerHitNpcHardSolid(const RECT *rcHit, player *me, const npc *NPC)
{
	long double xDiff = 0.0;
	long double yDiff = 0.0;
	int hit = 0;

	long double fx1 = 0.0;
	long double fx2 = 0.0;
	long double fy1 = 0.0;
	long double fy2 = 0.0;

	if (NPC->x <= me->x)
		xDiff = static_cast<long double>(me->x) - NPC->x;
	else
		xDiff = static_cast<long double>(NPC->x) - me->x;

	fx1 = xDiff;
	fx2 = NPC->hit.right;

	if (fx1 == 0.0)
		fx1 = 1.0;
	if (fx2 == 0.0)
		fx2 = 1.0;

	if (NPC->y <= me->y)
		yDiff = static_cast<long double>(me->y) - NPC->y;
	else
		yDiff = static_cast<long double>(NPC->y) - me->y;

	fy1 = yDiff;
	fy2 = NPC->hit.top;

	if (fy1 / fx1 <= fy2 / fx2)
	{
		if (me->y - rcHit->top < NPC->y + NPC->hit.bottom && me->y + rcHit->bottom > NPC->y - NPC->hit.top)
		{
			if (me->x - rcHit->left < NPC->x + NPC->hit.right && me->x - rcHit->left > NPC->x)
			{
				if (me->xm < NPC->xm)
					me->xm = NPC->xm;

				me->x = NPC->hit.right + NPC->x + rcHit->left;

				hit = leftWall;
			}

			if (me->x + rcHit->right > NPC->x - NPC->hit.right && me->x + rcHit->right < NPC->x)
			{
				if (me->xm > NPC->xm)
					me->xm = NPC->xm;

				me->x = NPC->x - NPC->hit.right - rcHit->right;

				hit |= rightWall;
			}
		}
	}
	else if (me->x - rcHit->left < NPC->x + NPC->hit.right && me->x + rcHit->right > NPC->x - NPC->hit.right)
	{
		if (me->y - rcHit->top < NPC->y + NPC->hit.bottom && me->y - rcHit->top > NPC->y)
		{
			if (me->ym >= NPC->ym)
			{
				if (me->ym < 0)
					me->ym = 0;
			}
			else
			{
				me->y = NPC->y + NPC->hit.bottom + rcHit->top + 0x200;

				me->ym = NPC->ym;
			}

			hit = ceiling;
		}

		if (me->y + rcHit->bottom > NPC->y - NPC->hit.top && me->y + rcHit->bottom < NPC->y + 0x600)
		{
			if (me->ym - NPC->ym > 1024)
				playSound(SFX_HitGround);

			if (me->unit == 1)
			{
				me->y = NPC->y - NPC->rect.top - rcHit->bottom + 0x200;
				hit |= ground;
			}

			if (NPC->bits & npc_bouncy)
			{
				me->ym = NPC->ym - 0x200;

				hit |= ground;
			}
			else if (!(me->flag & ground) && me->ym > NPC->ym)
			{
				me->y = NPC->y - NPC->hit.top - rcHit->bottom + 0x200;

				me->ym = NPC->ym;
				me->x += NPC->xm;

				hit |= ground;
			}
		}
	}
	return hit;
}

attrPure int playerHitNpcNonSolid(const RECT *rcHit, const player *me, const npc *NPC);

int playerHitNpcNonSolid(const RECT * /*rcHit*/, const player *me, const npc *NPC)
{
	if (NPC->direct != dirLeft)
	{
		if (me->x + 0x400 > NPC->x - NPC->hit.right
		        && me->x - 0x400 < NPC->x + NPC->hit.left
		        && me->y + 0x400 > NPC->y - NPC->hit.top
		        && me->y - 0x400 < NPC->y + NPC->hit.bottom)
		{
			return 1;
		}
	}

	if (me->x + 0x400 > NPC->x - NPC->hit.left
	        && me->x - 0x400 < NPC->x + NPC->hit.right
	        && me->y + 0x400 > NPC->y - NPC->hit.top
	        && me->y - 0x400 < NPC->y + NPC->hit.bottom)
	{
		return 1;
	}

	return 0;
}

void playerHitNpcs()
{
	player *me = &currentPlayer;
	const RECT *rcHit = &me->hit;

	if ((me->cond & player_visible) && !(me->cond & player_removed))
	{
		for (size_t i = 0; i < gNPC.size(); ++i)
		{
			if (gNPC[i].cond & npccond_alive)
			{
				int hit = 0;
				if (gNPC[i].bits & npc_solidSoft)
				{
					hit = playerHitNpcSoftSolid(rcHit, me, &gNPC[i]);

					me->flag |= hit;
				}
				else if (gNPC[i].bits & npc_solidHard)
				{
					hit = playerHitNpcHardSolid(rcHit, me, &gNPC[i]);

					me->flag |= hit;
				}
				else
					hit = playerHitNpcNonSolid(rcHit, me, &gNPC[i]);

				if (hit && gNPC[i].code_char == NPC_EXP)
				{
					playSound(SFX_GetEXP);
					giveWeaponExperience(gNPC[i].exp);
					gNPC[i].cond = 0;
				}
				if (hit && gNPC[i].code_char == NPC_Missile)
				{
					playSound(SFX_QuoteMissileGet);
					giveAmmo(gNPC[i].exp);
					gNPC[i].cond = 0;
				}

				if (hit && gNPC[i].code_char == NPC_Heart)
				{
					playSound(SFX_QuoteHeal);
					me->life += gNPC[i].exp;
					if (me->life > me->max_life)
						me->life = me->max_life;
					gNPC[i].cond = 0;
				}

				if (!(gGameFlags & 4) && hit && gNPC[i].bits & npc_eventTouch)
					startTscEvent(tsc, gNPC[i].code_event);

				if (gGameFlags & 2 && !(gNPC[i].bits & npc_interact))
				{
					if (gNPC[i].bits & npc_rearTop)
					{
						if (hit & rightWall && gNPC[i].xm < 0)
							me->damage(gNPC[i].damage);
						if (hit & leftWall && gNPC[i].xm > 0)
							me->damage(gNPC[i].damage);
						if (hit & ground && gNPC[i].ym < 0)
							me->damage(gNPC[i].damage);
						if (hit & ceiling && gNPC[i].ym > 0)
							me->damage(gNPC[i].damage);
					}
					else if (hit && gNPC[i].damage && !(gGameFlags & 4))
						me->damage(gNPC[i].damage);
				}

				if (!(gGameFlags & 4) && hit && me->cond & player_interact && gNPC[i].bits & npc_interact)
				{
					startTscEvent(tsc, gNPC[i].code_event);
					me->xm = 0;
					me->ques = false;
				}
			}
		}

		if (me->ques)
			createCaret(me->x, me->y, effect_ExclamationMark, 0);
	}
}

void playerHitBosses()
{
	player *me = &currentPlayer;
    const RECT *rcHit = &me->hit;

    if (me->cond & player_visible && !(me->cond & player_removed))
    {
        for (size_t i = 0; i < BOSSNPCS; ++i)
        {
            int hit = 0;
            if (gBossObj[i].cond & npccond_alive)
            {
                if (gBossObj[i].bits & npc_solidSoft)
                {
                    hit = playerHitNpcSoftSolid(rcHit, me, &gBossObj[i]);
                    me->flag |= hit;
                }
                else if (gBossObj[i].bits & npc_solidHard)
                {
                    hit = playerHitNpcHardSolid(rcHit, me, &gBossObj[i]);
                    me->flag |= hit;
                }
                else
                    hit = playerHitNpcNonSolid(rcHit, me, &gBossObj[i]);

                if (!(gGameFlags & 4) && hit && gBossObj[i].bits & npc_eventTouch)
                {
                    startTscEvent(tsc, gBossObj[i].code_event);
                    me->ques = 0;
                }

				if (gBossObj[i].bits & npc_rearTop)
				{
					if (hit & rightWall && gBossObj[i].xm < 0)
						me->damage(gBossObj[i].damage);
					if (hit & leftWall && gBossObj[i].xm > 0)
						me->damage(gBossObj[i].damage);
				}
				else if (hit && gBossObj[i].damage && !(gGameFlags & 4))
					me->damage(gBossObj[i].damage);

				if (!(gGameFlags & 4) && hit && me->cond & player_interact)
				{
					if (gBossObj[i].bits & npc_interact)
					{
						startTscEvent(tsc, gBossObj[i].code_event);

						me->xm = 0;
						me->ques = 0;
					}
				}
				if (gGameFlags & 2 && !(gNPC[i].bits & npc_interact))
				{
					if (gNPC[i].bits & npc_rearTop)
					{
						if (hit & rightWall && gNPC[i].xm < 0)
							me->damage(gNPC[i].damage);
						if (hit & leftWall && gNPC[i].xm > 0)
							me->damage(gNPC[i].damage);
						if (hit & ground && gNPC[i].ym < 0)
							me->damage(gNPC[i].damage);
						if (hit & ceiling && gNPC[i].ym > 0)
							me->damage(gNPC[i].damage);
					}
				}
			}
			else if (hit && gNPC[i].damage && !(gGameFlags & 4))
				me->damage(gNPC[i].damage);

			if (!(gGameFlags & 4) && hit && me->cond & player_interact)
			{
				if (gBossObj[i].bits & npc_interact)
				{
					startTscEvent(tsc, gBossObj[i].code_event);

					me->xm = 0;
					me->ques = 0;
				}
			}
		}
		if (me->ques)
			createCaret(me->x, me->y, effect_ExclamationMark);
	}
}
