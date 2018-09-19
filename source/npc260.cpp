#include "npc260.h"

#include <array>
#include "mathUtils.h"
#include "player.h"
#include "render.h"
#include "sound.h"
#include "level.h"

using std::array;

void npcAct263(npc *NPC) // Doctor (boss)
{
    array<RECT, 9> rcLeft, rcRight;

    rcLeft[0] = {0, 0, 24, 32};
    rcLeft[1] = {24, 0, 48, 32};
    rcLeft[2] = {48, 0, 72, 32};
    rcLeft[3] = {0, 0, 0, 0};
    rcLeft[4] = {72, 0, 96, 32};
    rcLeft[5] = {96, 0, 120, 32};
    rcLeft[6] = {120, 0, 144, 32};
    rcLeft[7] = {144, 0, 168, 32};
    rcLeft[8] = {264, 0, 288, 32};

    rcRight[0] = {0, 32, 24, 64};
    rcRight[1] = {24, 32, 48, 64};
    rcRight[2] = {48, 32, 72, 64};
    rcRight[3] = rcLeft[3];
    rcRight[4] = {72, 32, 96, 64};
    rcRight[5] = {96, 32, 120, 64};
    rcRight[6] = {120, 32, 144, 64};
    rcRight[7] = {144, 32, 168, 64};
    rcRight[8] = {264, 32, 288, 64};

	enum
	{
		init = 0,
		transforming = 2,
		fighting = 10,
		fireWaveShot = 20,
		bigBlast = 30,
		afterBlast = 32,
		teleportAway = 100,
		waitingToReappear = 102,
		reappear = 103,
		defeated = 500,
		flashing = 501,
	};

    switch (NPC->act_no)
    {
    case init:
        NPC->act_no = init + 1;
        NPC->y += tilesToUnits(0.5);
        NPC->ani_no = 3;
        break;

    case transforming:
        if (++NPC->act_wait / 2 & 1)
            NPC->ani_no = 0;
        else
            NPC->ani_no = 3;

        if (NPC->act_wait > 50)
            NPC->act_no = fighting;
        break;

    case fighting:
        NPC->ym += 0x80;
        NPC->bits |= npc_shootable;
        NPC->damage = 3;
        if (NPC->flag & ground)
        {
            NPC->act_no = fireWaveShot;
            NPC->act_wait = 0;
            NPC->ani_no = 0;
            NPC->count2 = NPC->life;
            NPC->facePlayer();
        }
        break;

    case fireWaveShot:
        if (++NPC->act_wait < 50 && NPC->life < NPC->count2 - 20)
            NPC->act_wait = 50;
        if (NPC->act_wait == 50)
        {
            NPC->facePlayer();
            NPC->ani_no = 4;
        }

        if (NPC->act_wait == 80)
        {
            NPC->ani_no = 5;
            playSound(SFX_SillyExplosion);
            if (NPC->direct != dirLeft)
            {
                createNpc(NPC_ProjectileDoctorRedWave, NPC->x + tilesToUnits(1), NPC->y, 0, 0, dirRight);
                createNpc(NPC_ProjectileDoctorRedWave, NPC->x + tilesToUnits(1), NPC->y, 0, 0, 0x400 + dirRight);
            }
            else
            {
                createNpc(NPC_ProjectileDoctorRedWave, NPC->x - tilesToUnits(1), NPC->y, 0, 0, dirLeft);
                createNpc(NPC_ProjectileDoctorRedWave, NPC->x - tilesToUnits(1), NPC->y, 0, 0, 0x400 + dirLeft);
            }
        }

        if (NPC->act_wait == 120)
            NPC->ani_no = 0;
        if (NPC->act_wait > 130 && NPC->life < NPC->count2 - 50)
            NPC->act_wait = 161;

        if (NPC->act_wait > 160)
        {
            NPC->act_no = teleportAway;
            NPC->ani_no = 0;
        }
        break;

    case bigBlast:
        NPC->act_no = bigBlast + 1;
        NPC->act_wait = 0;
        NPC->ani_no = 6;
        NPC->tgt_x = NPC->x;
        NPC->bits |= npc_shootable;
        // Fallthrough
    case bigBlast + 1:
        if (++NPC->act_wait / 2 & 1)
            NPC->x = NPC->tgt_x;
        else
            NPC->x = NPC->tgt_x + pixelsToUnits(1);

        if (NPC->act_wait > 50)
        {
            NPC->act_no = afterBlast;
            NPC->act_wait = 0;
            NPC->ani_no = 7;
            playSound(SFX_Lightning);

            for (size_t i = 8; i < 0x100; i += 0x10)
                createNpc(NPC_ProjectileDoctorRedBallSlowVanish, NPC->x, NPC->y, 2 * getCos(i), 2 * getSin(i));
        }
        break;

    case afterBlast:
        if (++NPC->act_wait > 50)
            NPC->act_no = teleportAway;
        break;

    case teleportAway:
        NPC->act_no = teleportAway + 1;
        NPC->bits &= ~npc_shootable;
        NPC->damage = 0;
        NPC->act_wait = 0;
        playSound(SFX_Teleport);
        break;

    case teleportAway + 1:
        NPC->act_wait += 2;
        if (NPC->act_wait > 16)
        {
            NPC->act_no = waitingToReappear;
            NPC->act_wait = 0;
            NPC->ani_no = 3;
            NPC->tgt_x = tilesToUnits(random(5, 35));
            NPC->tgt_y = tilesToUnits(random(5, 7));
        }
        break;

    case waitingToReappear:
        if (++NPC->act_wait > 40)
        {
            NPC->act_no = reappear;
            NPC->act_wait = 16;
            NPC->ani_no = 2;
            NPC->ym = 0;
            NPC->x = NPC->tgt_x;
            NPC->y = NPC->tgt_y;
            NPC->facePlayer();
        }
        break;

    case reappear:
        NPC->act_wait -= 2;
        if (NPC->act_wait <= 0)
        {
            NPC->bits |= npc_shootable;
            NPC->damage = 3;
            if (NPC->count1 >= 3)
            {
                NPC->count1 = 0;
                NPC->act_no = bigBlast;
            }
            else
            {
                ++NPC->count1;
                NPC->act_no = fighting;
            }
        }
        break;

    case defeated:
        NPC->bits &= ~npc_shootable;
        NPC->ani_no = 6;
        NPC->ym += 0x10;

        if (NPC->flag & ground)
        {
            NPC->act_no = teleportAway + 1;
            NPC->act_wait = 0;
            NPC->tgt_x = NPC->x;
            NPC->facePlayer();
        }
        break;

    case defeated + 1:
        NPC->facePlayer();
        NPC->ani_no = 8;
        if (++NPC->act_wait / 2 & 1)
            NPC->x = NPC->tgt_x;
        else
            NPC->x = NPC->tgt_x + pixelsToUnits(1);
    }

    if (NPC->act_no >= fighting)
    {
        if (NPC->act_no == waitingToReappear)
        {
            superXPos = NPC->tgt_x;
            superYPos = NPC->tgt_y;
        }
        else
        {
            superXPos = NPC->x;
            superYPos = NPC->y;
        }
    }

    NPC->doGravity(0, 0x5FF);
    NPC->x += NPC->xm;
    NPC->y += NPC->ym;

    NPC->doRects(rcLeft, rcRight);

    if (NPC->act_no != teleportAway + 1 && NPC->act_no != reappear)
        NPC->view.top = tilesToUnits(1);
    else
    {
        NPC->rect.top += NPC->act_wait;
        NPC->rect.bottom -= NPC->act_wait;
        NPC->view.top = pixelsToUnits(16 - NPC->act_wait);
    }
}

void npcAct267(npc *NPC) // Muscle Doctor (boss)
{
    /**

        TBD : HEY THIS ISN'T FINISHED SO PLS DO SO (also implement the first form of the doctor
                                                before doing so cos it doesn't work w/o him anyway)

    */

    enum
    {
        init = 0,
        stand = 5,
        defeated = 500,
        dissolve = 510,
    };

    array<RECT, 10> rcLeft, rcRight;

    rcLeft[0] = {0, 0, 0, 0};
    rcLeft[1] = {0, 64, 40, 112};
    rcLeft[2] = {40, 64, 80, 112};
    rcLeft[3] = {80, 64, 120, 112};
    rcLeft[4] = {120, 64, 160, 112};
    rcLeft[5] = {160, 64, 200, 112};
    rcLeft[6] = {200, 64, 240, 112};
    rcLeft[7] = {240, 64, 280, 112};
    rcLeft[8] = {280, 64, 320, 112};
    rcLeft[9] = {0, 160, 40, 208};

    rcRight[0] = rcLeft[0];
    rcRight[1] = {0, 112, 40, 160};
    rcRight[2] = {40, 112, 80, 160};
    rcRight[3] = {80, 112, 120, 160};
    rcRight[4] = {120, 112, 160, 160};
    rcRight[5] = {160, 112, 200, 160};
    rcRight[6] = {200, 112, 240, 160};
    rcRight[7] = {240, 112, 280, 160};
    rcRight[8] = {280, 112, 320, 160};
    rcRight[9] = {40, 160, 80, 208};

    switch (NPC->act_no)
    {
    case init:
        if (superXPos <= currentPlayer.x)
            NPC->direct = dirRight;
        else
            NPC->direct = dirLeft;

        if (NPC->direct != dirLeft)
            NPC->x = superXPos + pixelsToUnits(6);
        else
            NPC->x = superXPos - pixelsToUnits(6);
        NPC->y = superYPos;
        // Fallthrough
    case 1:
    case 2:
        NPC->ym += 0x80;
        if (++NPC->act_wait / 2 & 1)
            NPC->ani_no = 0;
        else
            NPC->ani_no = 3;
        break;

    case stand: // Unused afaik
        NPC->act_no = stand + 1;
        NPC->ani_no = 1;
        NPC->ani_wait = 0;
        // Fallthrough
    case stand + 1:
        NPC->ym += 0x80;
        NPC->animate(40, 1, 2);
        break;

    case 7:
        NPC->act_no = 8;
        NPC->act_wait = 0;
        NPC->ani_no = 3;
        // Fallthrough
    case 8:
        NPC->ym += 0x40;
        if (++NPC->act_wait > 40)
            NPC->act_no = 10;
        break;
    }

    NPC->doGravity(0, 0x5FF);
    NPC->x += NPC->xm;
    NPC->y += NPC->ym;

    NPC->doRects(rcLeft, rcRight);
}

void npcAct278(npc * NPC)
{
	constexpr RECT rcLittleMan[2] = { {0, 120, 8, 128}, {8, 120, 16, 128} };
	constexpr RECT rcLittleWoman[2] = { {16, 120, 24, 128}, {24, 120, 32, 128} };
	constexpr RECT rcLittleChild[2] = { {32, 120, 40, 128}, {40, 120, 48, 128} };

	switch (NPC->act_no)
	{
	case 0:
		NPC->act_no = 1;
		NPC->ani_no = 0;
		NPC->ani_wait = 0;
		NPC->xm = 0;
		// Fallthrough
	case 1:
		if (random(0, 60) == 1)
		{
			NPC->act_no = 2;
			NPC->act_wait = 0;
			NPC->ani_no = 1;
		}
		if (random(0, 60) == 1)
		{
			NPC->act_no = 10;
			NPC->act_wait = 0;
			NPC->ani_no = 1;
		}
		break;

	case 2:
		if (++NPC->act_wait > 8)
		{
			NPC->act_no = 1;
			NPC->ani_no = 0;
		}
		break;

	case 10:
		NPC->act_no = 11;
		NPC->act_wait = random(0, 16);
		NPC->ani_no = 0;
		NPC->ani_wait = 0;
		NPC->direct = random(0, 1) ? dirLeft : dirRight;
		// Fallthrough
	case 11:
		if (NPC->direct != dirLeft || !(NPC->flag & leftWall))
		{
			if (NPC->direct == dirRight && NPC->flag & rightWall)
				NPC->direct = dirLeft;
		}
		else
			NPC->direct = dirRight;

		if (NPC->direct != dirLeft)
			NPC->xm = 0x100;
		else
			NPC->xm = -0x100;

		if (++NPC->ani_wait > 4)
		{
			NPC->ani_wait = 0;
			++NPC->ani_no;
		}
		if (NPC->ani_no > 1)
			NPC->ani_no = 0;

		if (++NPC->act_wait > 0x20)
			NPC->act_no = 0;
		break;

	default:
		break;
	}

	NPC->ym += 0x20;
	if (NPC->ym > 0x5FF)
		NPC->ym = 0x5FF;

	NPC->x += NPC->xm;
	NPC->y += NPC->ym;

	if (NPC->code_event == 200)
		NPC->rect = rcLittleMan[NPC->ani_no];
	else if (NPC->code_event == 210)
		NPC->rect = rcLittleWoman[NPC->ani_no];
	else
		NPC->rect = rcLittleChild[NPC->ani_no];
}
