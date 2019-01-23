#include "weapons.h"
#include "shootFunctions.h"

#include <string>
#include <cstring>
#include <SDL_messagebox.h>
#include "player.h"
#include "sound.h"
#include "caret.h"
#include "bullet.h"
#include "game.h"

//Shoot functions
weaponShoot shootFunctions[14] =
{
	static_cast<weaponShoot>(nullptr),
	&shootSnake,
	&shootPolarStar,
	&shootFireball,
	&shootMachineGun,
	&shootMissile,
	static_cast<weaponShoot>(nullptr),
	&pewBubble,
	static_cast<weaponShoot>(nullptr),
	&throwBlade,
	&launchSuperMissile,
	static_cast<weaponShoot>(nullptr),
	&shootNemesis,
	&shootSpur,
};

//Weapon Levels
WEAPONEXP weaponLevels[14] =
{
	{{ 0,	0,	100 }},
	{{ 30,	40,	16 }},
	{{ 10,	20,	10 }},
	{{ 10,	20,	20 }},
	{{ 30,	40,	10 }},
	{{ 10,	20,	10 }},
	{{ 10,	20,	30 }},
	{{ 10,	20,	5 }},
	{{ 10,	20,	100 }},
	{{ 30,	60,	0 }},
	{{ 30,	60,	10 }},
	{{ 10,	20,	100 }},
	{{ 1,	1,	1 }},
	{{ 40,	60,	200 }},
};

WEAPON weapons[WEAPONS];

RECT weaponRect;
int selectedWeapon;
int weaponShiftX;
int weaponExpFlash;

void initWeapons()
{
	memset(weapons, 0, sizeof(weapons));
	selectedWeapon = 0;
	weaponShiftX = 32;
}

//Functions for shooting
void actWeapon()
{
	if (_empty)
		--_empty;

	if (!(currentPlayer.cond & player_removed) && weapons[selectedWeapon].code)
	{
		if (shootFunctions[weapons[selectedWeapon].code] != nullptr)
			shootFunctions[weapons[selectedWeapon].code](weapons[selectedWeapon].level);
		else if (debugFlags & notifyOnNotImplemented)
		{
			static bool wasNotifiedAbout[_countof(shootFunctions)] = { false };

			if (wasNotifiedAbout[weapons[selectedWeapon].code])
				return;

			wasNotifiedAbout[weapons[selectedWeapon].code] = true;
			std::string msg = "Weapon " + std::to_string(weapons[selectedWeapon].code) + " is not implemented.";
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Missing Weapon", msg.c_str(), nullptr);
		}
	}
}

void giveWeaponAmmo(int num)
{
	weapons[selectedWeapon].num += num;
	if (weapons[selectedWeapon].num > weapons[selectedWeapon].max_num)
		weapons[selectedWeapon].num = weapons[selectedWeapon].max_num;
}

int useWeaponAmmo(int num)
{
	//Some checks
	if (!weapons[selectedWeapon].max_num)
		return 1;
	if (!weapons[selectedWeapon].num)
		return 0;

	//Use ammo
	weapons[selectedWeapon].num -= num;
	if (weapons[selectedWeapon].num < 0)
		weapons[selectedWeapon].num = 0;
	return 1;
}

void giveAmmo(int ammoToRefill)
{
	int i;
	for (i = 0; i < 8 && weapons[i].code != weapon_MissileLauncher && weapons[i].code != weapon_SuperMissileLauncher; ++i);
	if (i != 8)
	{
		weapons[i].num += ammoToRefill;
		if (weapons[i].num > weapons[i].max_num)
			weapons[i].num = weapons[i].max_num;
	}
}

bool weaponMaxExp()
{
	return weapons[selectedWeapon].level == 3
	       && weapons[selectedWeapon].exp >= weaponLevels[weapons[selectedWeapon].code].exp[2];
}

int weaponBullets(int arms_code)
{
	int count = 0;
	for (size_t i = 0; i < bullets.size(); ++i)
	{
		if (bullets[i].cond & 0x80 && bullets[i].weapon == arms_code)
			++count;
	}
	return count;
}

//TSC functions
int tradeWeapons(int code1, int code2, int max_num)
{
	int i;
	for (i = 0; i < WEAPONS && weapons[i].code != code1; ++i);
	if (i == WEAPONS)
		return 0;

	weapons[i].level = 1;
	weapons[i].code = code2;
	weapons[i].max_num += max_num;
	weapons[i].num += max_num;
	weapons[i].exp = 0;
	return 1;
}

int giveWeapon(int code, int maxAmmo)
{
	int i;
	for (i = 0; i < WEAPONS && weapons[i].code != code && weapons[i].code; ++i)
        ;
	if (i == WEAPONS)
		return 0;

	//Ensure past data is erased
	if (!weapons[i].code)
	{
		memset(&weapons[i], 0, sizeof(WEAPON));
		weapons[i].level = 1;
	}

	//Set weapon and ammo
	weapons[i].code = code;
	weapons[i].max_num += maxAmmo;
	weapons[i].num += maxAmmo;

	if (weapons[i].num > weapons[i].max_num)
		weapons[i].num = weapons[i].max_num;
	return 1;
}

int removeWeapon(int code)
{
	int i;
	for (i = 0; i < WEAPONS && weapons[i].code != code; ++i);
	if (i == WEAPONS)
		return 0;

	//Push back all weapons ahead
	int ia;
	for (ia = i + 1; ia < 8; ++ia)
		memcpy(&weapons[ia - 1], &weapons[ia], sizeof(WEAPON));

	//Clear farthest weapon and select first weapon
	weapons[ia - 1].code = 0;
	selectedWeapon = 0;
	return 1;
}

void clearWeaponExperience()
{
	for (auto& i : weapons)
	{
		i.level = 1;
		i.exp = 0;
	}
}

void maxWeaponAmmo()
{
	for (auto& i : weapons)
		i.num = i.max_num;
}

bool checkWeapon(int code)
{
	for (const auto& i : weapons)
	{
		if (i.code == code)
			return true;
	}
	return false;
}

void giveWeaponExperience(int x)
{
	int lv = weapons[selectedWeapon].level - 1; // [esp+18h] [ebp-10h]
	const int arms_code = weapons[selectedWeapon].code; // [esp+1Ch] [ebp-Ch]
	weapons[selectedWeapon].exp += x;

	if (lv == 2)
	{
		if (weapons[selectedWeapon].exp >= weaponLevels[arms_code].exp[2])
		{
			weapons[selectedWeapon].exp = weaponLevels[arms_code].exp[2];

			//Give a whimsical star
			if (currentPlayer.equip & equip_whimsicalStar && currentPlayer.star < 3)
				currentPlayer.star++;
		}
	}
	else
	{
		while (lv <= 1)
		{
			if (weapons[selectedWeapon].exp >= weaponLevels[arms_code].exp[lv])
			{
				++weapons[selectedWeapon].level;
				weapons[selectedWeapon].exp = 0;

				if (weapons[selectedWeapon].code != 13)
				{
					playSound(SFX_LevelUp);
					createCaret(currentPlayer.x, currentPlayer.y, effect_LevelUpOrDown);
				}
			}

			++lv;
		}
	}

	if (weapons[selectedWeapon].code == 13)
	{
		currentPlayer.exp_wait = 10;
	}
	else
	{
		currentPlayer.exp_count += x;
		currentPlayer.exp_wait = 30;
	}
}

void resetSelectedWeaponLevel()
{
	weapons[selectedWeapon].level = 1;
	weapons[selectedWeapon].exp = 0;
}

void resetSpurCharge()
{
	spurCharge = 0;
	if (weapons[selectedWeapon].code == weapon_Spur)
		resetSelectedWeaponLevel();
}

//Rotate weapon functions
int rotateWeaponRight()
{
	int weaponNo;

	for (weaponNo = 0; weaponNo < WEAPONS && weapons[weaponNo].code != 0; ++weaponNo);
	if (!weaponNo)
		return 0;

	resetSpurCharge();

	//Rotate to the right, wrap around
	++selectedWeapon;
	while (selectedWeapon < weaponNo && !weapons[selectedWeapon].code)
		++selectedWeapon;
	if (selectedWeapon == weaponNo)
		selectedWeapon = 0;

	//Do effects
	weaponShiftX = 32;
	playSound(SFX_SwitchWeapon);

	return weapons[weaponNo].code;
}

int rotateWeaponLeft()
{
	int weaponNo;

	for (weaponNo = 0; weaponNo < WEAPONS && weapons[weaponNo].code != 0; ++weaponNo);
	if (!weaponNo)
		return 0;

	resetSpurCharge();

	//Rotate to the left, wrap around
	--selectedWeapon;
	if (selectedWeapon < 0)
		selectedWeapon = weaponNo - 1;
	while (selectedWeapon < weaponNo && !weapons[selectedWeapon].code)
		--selectedWeapon;

	//Do effects
	weaponShiftX = 0;
	playSound(SFX_SwitchWeapon);

	return weapons[weaponNo].code;
}

