#include "bulletCollision.h"
#include "bullet.h"
#include "boss.h"
#include "sound.h"
#include "main.h"
#include "caret.h"
#include "stage.h"
#include "game.h"
#include "mathUtils.h"
#include "player.h"
#include "script.h"

void bulletVanish(bullet *bul)
{
	if (bul->code_bullet != 37 && bul->code_bullet != 38 && bul->code_bullet != 39)
		playSound(SFX_ShotHitWall);
	else
		createCaret(bul->x, bul->y, effect_RisingDisc, dirUp);

	bul->cond = 0;
	createCaret(bul->x, bul->y, effect_RisingDisc, dirRight);
}

int bulletJudgeBlock(size_t x, size_t y, bullet *bul) //For judging breakable blocks
{
	int hit = 0;

	//Check if hit tile
	if (bul->x - bul->blockXL < static_cast<int>(2 * x + 1) << 12
		&& bul->blockXL + bul->x > static_cast<int>(2 * x - 1) << 12
		&& bul->y - bul->blockYL < static_cast<int>(2 * y + 1) << 12
		&& bul->blockYL + bul->y > static_cast<int>(2 * y - 1) << 12)
	{
		hit = 0x200;
	}

	//Destroy breakable blocks
	if (hit && bul->bbits & (bullet_breakBlocks | bullet_pierceBlocks) && getTileAttribute(x, y) == 0x43)
	{
		//Delete if not piercing blocks
		if (!(bul->bbits & bullet_pierceBlocks))
			bul->cond = 0;

		//Do effects when breaking block
		createCaret(bul->x, bul->y, effect_RisingDisc);
		playSound(SFX_DestroyBreakableBlock);

		for (int i = 0; i < 4; ++i)
			createNpc(NPC_Smoke, tilesToUnits(x), tilesToUnits(y), random(-512, 512), random(-512, 512));

		//Shift tile -1
		shiftTile(x, y);
	}

	return hit;
}

int bulletJudgeBlock2(int x, int y, const uint8_t *atrb, bullet *bul) //For judging actual solid blocks
{
	if (atrb == nullptr)
		doCustomError("atrb was nullptr in bulletJudgeBlock2");

	int block[4];
	int workX;
	int workY;
	int hit = 0;

	//Get tiles that are considered solid nearby
	if (bul->bbits & bullet_pierceBlocks)
	{
		//Pierce blocks flag, doesn't check for breakable block
		for (int i = 0; i < 4; ++i)
			block[i] = atrb[i] == 0x41 || atrb[i] == 0x61;
	}
	else
	{
		//Checks for breakable block as well
		for (int i = 0; i < 4; ++i)
			block[i] = atrb[i] == 0x41 || atrb[i] == 0x43 || atrb[i] == 0x61;
	}

	//Get tile position things??
	workX = (2 * x + 1) << 12;
	workY = (2 * y + 1) << 12;

	//Check if hitting left wall
	if (block[0] && block[2])
	{
		if (bul->x - bul->blockXL < workX)
			hit |= leftWall;
	}
	else if (!block[0] || block[2])
	{
		if (!block[0] && block[2] && bul->x - bul->blockXL < workX && bul->blockYL + bul->y > workY + pixelsToUnits(3))
			hit |= leftWall;
	}
	else if (bul->x - bul->blockXL < workX && bul->y - bul->blockYL < workY - pixelsToUnits(3))
		hit |= leftWall;

	//Check if hitting right wall
	if (block[1] && block[3])
	{
		if (bul->x + bul->blockXL > workX)
			hit |= rightWall;
	}
	else if (!block[1] || block[3])
	{
		if (!block[1] && block[3] && bul->x + bul->blockXL > workX && bul->blockYL + bul->y > workY + pixelsToUnits(3))
			hit |= rightWall;
	}
	else if (bul->x + bul->blockXL > workX && bul->y - bul->blockYL < workY - pixelsToUnits(3))
	{
		hit |= rightWall;
	}

	//Check if hitting ceiling
	if (block[0] && block[1])
	{
		if (bul->y - bul->blockYL < workY)
			hit |= ceiling;
	}
	else if (!block[0] || block[1])
	{
		if (!block[0] && block[1] && bul->y - bul->blockYL < workY && bul->blockXL + bul->x > workX + pixelsToUnits(3))
			hit |= ceiling;
	}
	else if (bul->y - bul->blockYL < workY && bul->x - bul->blockXL < workX - pixelsToUnits(3))
		hit |= ceiling;

	//Check if hitting ground
	if (block[2] && block[3])
	{
		if (bul->y + bul->blockYL > workY)
			hit |= ground;
	}
	else if (!block[2] || block[3])
	{
		if (!block[2] && block[3] && bul->y + bul->blockYL > workY && bul->blockXL + bul->x > workX + pixelsToUnits(3))
			hit |= ground;
	}
	else if (bul->y + bul->blockYL > workY && bul->x - bul->blockXL < workX - pixelsToUnits(3))
		hit |= ground;

	//Push out of walls if solid
	if (bul->bbits & bullet_goThroughWalls)
	{
		if (hit & leftWall)
			bul->x = workX + bul->blockXL;
		else if (hit & rightWall)
			bul->x = workX - bul->blockXL;
		else if (hit & ceiling)
			bul->y = workY + bul->blockYL;
		else if (hit & ground)
			bul->y = workY - bul->blockYL;
	}
	else if (hit & (leftWall | ceiling | rightWall | ground))
	{
		//Break if hitting wall
		bulletVanish(bul);
	}

	return hit;
}

int bulletJudgeTriangleA(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x > (2 * x - 1) << 12
		&& bul->y - pixelsToUnits(2) < tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(4)
		&& bul->y + pixelsToUnits(2) > (2 * y - 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(6);
		else
			bulletVanish(bul);

		hit = 0x82;
	}

	return hit;
}

int bulletJudgeTriangleB(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x > (2 * x - 1) << 12
		&& bul->y - pixelsToUnits(2) < tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(4)
		&& bul->y + pixelsToUnits(2) > (2 * y - 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(2);
		else
			bulletVanish(bul);

		hit = 0x82;
	}

	return hit;
}

int bulletJudgeTriangleC(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x > (2 * x - 1) << 12
		&& bul->y - pixelsToUnits(2) < tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(4)
		&& bul->y + pixelsToUnits(2) > (2 * y - 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(2);
		else
			bulletVanish(bul);

		hit = 0x42;
	}

	return hit;
}

int bulletJudgeTriangleD(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x > (2 * x - 1) << 12
		&& bul->y - pixelsToUnits(2) < tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(4)
		&& bul->y + pixelsToUnits(2) > (2 * y - 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(6);
		else
			bulletVanish(bul);

		hit = 0x42;
	}

	return hit;
}

int bulletJudgeTriangleE(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x - pixelsToUnits(1) > (2 * x - 1) << 12
		&& bul->y + pixelsToUnits(2) > tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(4)
		&& bul->y - pixelsToUnits(2) < (2 * y + 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(6);
		else
			bulletVanish(bul);

		hit = 0x28;
	}

	return hit;
}

int bulletJudgeTriangleF(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x - pixelsToUnits(1) > (2 * x - 1) << 12
		&& bul->y + pixelsToUnits(2) > tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(4)
		&& bul->y - pixelsToUnits(2) < (2 * y + 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) + (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(2);
		else
			bulletVanish(bul);

		hit = 0x28;
	}

	return hit;
}

int bulletJudgeTriangleG(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x - pixelsToUnits(1) >(2 * x - 1) << 12
		&& bul->y + pixelsToUnits(2) > tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(4)
		&& bul->y - pixelsToUnits(2) < (2 * y + 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 + pixelsToUnits(2);
		else
			bulletVanish(bul);

		hit = 0x18;
	}

	return hit;
}

int bulletJudgeTriangleH(int x, int y, bullet *bul)
{
	int hit = 0;

	if (bul->x < (2 * x + 1) << 12
		&& bul->x - 0x200 > (2 * x - 1) << 12
		&& bul->y + pixelsToUnits(2) > tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(4)
		&& bul->y - pixelsToUnits(2) < (2 * y + 1) << 12)
	{
		if (bul->bbits & bullet_goThroughWalls)
			bul->y = tilesToUnits(y) - (tilesToUnits(-1) * x + bul->x) / 2 - pixelsToUnits(6);
		else
			bulletVanish(bul);

		hit = 0x18;
	}

	return hit;
}

//Main functions
void bulletHitMap()
{
	for (size_t i = 0; i < gBullets.size(); i++)
	{
		bullet *bul = &gBullets[i];

		if (bul->cond & 0x80)
		{
			//Get offset positions for tile checking
			const size_t x = unitsToTiles(bul->x);
			const size_t y = unitsToTiles(bul->y);

			uint8_t atrb[4];

			//Get tile attributes nearby
			atrb[0] = getTileAttribute(x, y);
			atrb[1] = getTileAttribute(x + 1, y);
			atrb[2] = getTileAttribute(x, y + 1);
			atrb[3] = getTileAttribute(x + 1, y + 1);

			//Clear previous collision flags
			bul->flag = 0;

			if (!(bul->bbits & bullet_ignoreSolid))
			{
				for (int j = 0; j < 4; ++j) //Go through each tile
				{
					if (gBullets[i].cond & 0x80)
					{
						constexpr int offx[4] = { 0, 1, 0, 1 };
						constexpr int offy[4] = { 0, 0, 1, 1 };

						switch (atrb[j])
						{
						case 0x41:
						case 0x43:
						case 0x44:
						case 0x61:
						case 0x64:
							bul->flag |= bulletJudgeBlock(x + offx[j], y + offy[j], bul);
							break;

						case 0x50u:
						case 0x70u:
							bul->flag |= bulletJudgeTriangleA(x + offx[j], y + offy[j], bul);
							break;
						case 0x51u:
						case 0x71u:
							bul->flag |= bulletJudgeTriangleB(x + offx[j], y + offy[j], bul);
							break;
						case 0x52u:
						case 0x72u:
							bul->flag |= bulletJudgeTriangleC(x + offx[j], y + offy[j], bul);
							break;
						case 0x53u:
						case 0x73u:
							bul->flag |= bulletJudgeTriangleD(x + offx[j], y + offy[j], bul);
							break;
						case 0x54u:
						case 0x74u:
							bul->flag |= bulletJudgeTriangleE(x + offx[j], y + offy[j], bul);
							break;
						case 0x55u:
						case 0x75u:
							bul->flag |= bulletJudgeTriangleF(x + offx[j], y + offy[j], bul);
							break;
						case 0x56u:
						case 0x76u:
							bul->flag |= bulletJudgeTriangleG(x + offx[j], y + offy[j], bul);
							break;
						case 0x57u:
						case 0x77u:
							bul->flag |= bulletJudgeTriangleH(x + offx[j], y + offy[j], bul);
							break;

						default:
							break;
						}
					}
				}

				//Collide with blocks with solitity (mainly for fireball, and checking if a bullet hit a wall)
				bul->flag |= bulletJudgeBlock2(x, y, atrb, bul);
			}
		}
	}
}

void bulletHitNpcs()
{
	for (size_t n = 0; n < gNPC.size(); n++)
	{
		if (gNPC[n].cond & npccond_alive && (!(gNPC[n].bits & npc_shootable) || !(gNPC[n].bits & npc_interact)))
		{
			for (size_t i = 0; ; i++)
			{
				//If at end of bullets, kill npc if dead flag is on
				if (i >= gBullets.size())
				{
					if (gNPC[n].cond & 8)
						killNpc(&gNPC[n], true);
					break;
				}

				bullet *bul = &gBullets[i];

				//Check if the bullet is tangible
				if (bul->cond & 0x80 && bul->damage != -1)
				{
					bool bHit = false;

					if (gNPC[n].bits & npc_shootable //Check for shootable npc collision
						&& gNPC[n].x - gNPC[n].hit.right < bul->x + bul->enemyXL
						&& gNPC[n].x + gNPC[n].hit.right > bul->x - bul->enemyXL
						&& gNPC[n].y - gNPC[n].hit.top < bul->y + bul->enemyYL
						&& gNPC[n].y + gNPC[n].hit.bottom > bul->y - bul->enemyYL)
						bHit = true;
					else if (gNPC[n].bits & npc_invulnerable //Check for collision with a specifically not shootable npc
						&& gNPC[n].x - gNPC[n].hit.right < bul->x + bul->blockXL
						&& gNPC[n].x + gNPC[n].hit.right > bul->x - bul->blockXL
						&& gNPC[n].y - gNPC[n].hit.top < bul->y + bul->blockYL
						&& gNPC[n].y + gNPC[n].hit.bottom > bul->y - bul->blockYL)
						bHit = true;

					if (bHit)
					{
						if (gNPC[n].bits & npc_shootable)
						{
							//NPC takes damage
							gNPC[n].life -= bul->damage;

							if (gNPC[n].life > 0)
							{
								//Shock if not already shocked for 2 frames
								if (gNPC[n].shock < 14)
								{
									for (int j = 0; j < 3; ++j)
										createCaret((bul->x + gNPC[n].x) / 2, (bul->y + gNPC[n].y) / 2, effect_RedDamageRings);

									playSound(gNPC[n].hit_voice);
									gNPC[n].shock = 16;
								}

								//Add to valueview damage
								if (gNPC[n].bits & npc_showDamage)
									gNPC[n].damage_view -= bul->damage;
							}
							else
							{
								//Keep life at 0 rather than some negative number (although this doesn't matter)
								gNPC[n].life = 0;

								//Hide boss healthbar if it's focused on this npc
								if (gBossLife.flag && gBossLife.pLife == &gNPC[n].life)
									gBossLife.flag = 0;

								//Add to valueview damage
								if (gNPC[n].bits & npc_showDamage)
									gNPC[n].damage_view -= bul->damage;

								//Either run event if the run event on death flag's set, or die
								if (gCurrentPlayer.cond & npccond_alive && gNPC[n].bits & npc_eventDie)
									startTscEvent(gTsc, gNPC[n].code_event);
								else
									gNPC[n].cond |= 8;
							}
						}
						else if (bul->code_bullet != 13
							&& bul->code_bullet != 14
							&& bul->code_bullet != 15
							&& bul->code_bullet != 28
							&& bul->code_bullet != 29
							&& bul->code_bullet != 30
							&& !(bul->bbits & 0x10))
						{
							//Break if hitting a non-shootable NPC
							createCaret((bul->x + gNPC[n].x) / 2, (bul->y + gNPC[n].y) / 2, effect_RisingDisc, dirRight);
							playSound(SFX_ShotHitInvulnerableEntity);
							bul->life = 0;
							continue;
						}

						//I think this is for the spur maybe?
						--bul->life;
					}
				}
			}
		}
	}
}

void bulletHitBoss()
{
	for (size_t n = 0; n < BOSSNPCS; n++)
	{
		if (gBossObj[n].cond & npccond_alive && (!(gBossObj[n].bits & npc_shootable) || !(gBossObj[n].bits & npc_interact)))
		{
			for (size_t i = 0; i < gBullets.size(); i++)
			{
				bullet *bul = &gBullets[i];

				//Check if the bullet is tangible
				if (bul->cond & 0x80 && bul->damage != -1)
				{
					bool bHit = false;

					if (gBossObj[n].bits & npc_shootable //Check for shootable npc collision
						&& gBossObj[n].x - gBossObj[n].hit.right < bul->x + bul->enemyXL
						&& gBossObj[n].x + gBossObj[n].hit.right > bul->x - bul->enemyXL
						&& gBossObj[n].y - gBossObj[n].hit.top < bul->y + bul->enemyYL
						&& gBossObj[n].y + gBossObj[n].hit.bottom > bul->y - bul->enemyYL)
						bHit = true;
					else if (gBossObj[n].bits & npc_invulnerable //Check for collision with a specifically not shootable npc
						&& gBossObj[n].x - gBossObj[n].hit.right < bul->x + bul->blockXL
						&& gBossObj[n].x + gBossObj[n].hit.right > bul->x - bul->blockXL
						&& gBossObj[n].y - gBossObj[n].hit.top < bul->y + bul->blockYL
						&& gBossObj[n].y + gBossObj[n].hit.bottom > bul->y - bul->blockYL)
						bHit = true;

					if (bHit)
					{
						if (gBossObj[n].bits & npc_shootable)
						{
							//Something
							int bos_;

							if (gBossObj[n].cond & npccond_dmgboss)
								bos_ = 0;
							else
								bos_ = n;

							//NPC takes damage
							gBossObj[bos_].life -= bul->damage;

							if (gBossObj[bos_].life > 0)
							{
								//Shock if not already shocked for 2 frames
								if (gBossObj[bos_].shock < 14)
								{
									for (int j = 0; j < 3; ++j)
										createCaret((bul->x + gBossObj[bos_].x) / 2, (bul->y + gBossObj[bos_].y) / 2, effect_RedDamageRings);

									playSound(gBossObj[bos_].hit_voice);
								}

								gBossObj[n].shock = 8;
								gBossObj[bos_].shock = 8;
								gBossObj[bos_].damage_view -= bul->damage;
							}
							else
							{
								//what the fuck you the pixel
								gBossObj[bos_].life = bos_;

								//Either run event if the run event on death flag's set, or die
								if (gCurrentPlayer.cond & 0x80 && gBossObj[bos_].bits & npc_eventDie)
								{
									startTscEvent(gTsc, gBossObj[bos_].code_event);
								}
								else
								{
									playSound(gBossObj[bos_].destroy_voice);

									switch (gBossObj[bos_].size)
									{
									case 1:
										createSmokeLeft(gBossObj[bos_].x, gBossObj[bos_].y, gBossObj[bos_].view.right, 4);
										break;
									case 2:
										createSmokeLeft(gBossObj[bos_].x, gBossObj[bos_].y, gBossObj[bos_].view.right, 8);
										break;
									case 3:
										createSmokeLeft(gBossObj[bos_].x, gBossObj[bos_].y, gBossObj[bos_].view.right, 16);
										break;
									}

									gBossObj[bos_].cond = 0;
								}
							}
						}
						else if (bul->code_bullet != 13
							&& bul->code_bullet != 14
							&& bul->code_bullet != 15
							&& bul->code_bullet != 28
							&& bul->code_bullet != 29
							&& bul->code_bullet != 30
							&& !(bul->bbits & 0x10))
						{
							//Break if hitting a non-shootable NPC
							createCaret((bul->x + gBossObj[n].x) / 2, (bul->y + gBossObj[n].y) / 2, effect_RisingDisc, dirRight);
							playSound(SFX_ShotHitInvulnerableEntity);
							bul->life = 0;
							continue;
						}
						--bul->life;
					}
				}
			}
		}
	}
}
