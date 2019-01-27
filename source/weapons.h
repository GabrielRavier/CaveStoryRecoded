#pragma once

#include "common.h"

struct WEAPONEXP
{
	int exp[3];
};

extern WEAPONEXP gWeaponLevels[14];

struct WEAPON
{
	int code;
	int level;
	int exp;
	int max_num;
	int num;
};

using weaponShoot = void(*)(int);

constexpr auto WEAPONS = 8;

extern WEAPON gWeapons[WEAPONS];
extern RECT gWeaponRect;
extern int gSelectedWeapon;
extern int gWeaponShiftX;
extern int gWeaponExpFlash;

void initWeapons();

void actWeapon();

int useWeaponAmmo(int num);
void giveAmmo(int ammoToRefill);
void giveWeaponAmmo(int amount);
attrPure bool weaponMaxExp();
attrPure int weaponBullets(int arms_code);

int giveWeapon(int code, int max_num);
int removeWeapon(int code);
int tradeWeapons(int code1, int code2, int max_num);
void clearWeaponExperience();
void maxWeaponAmmo();
attrPure bool checkWeapon(int code);
void giveWeaponAmmo(int num);

void giveWeaponExperience(int x);

void resetSelectedWeaponLevel();

int rotateWeaponRight();
int rotateWeaponLeft();

enum weaponsNames
{
	weapon_Snake = 1,
	weapon_PolarStar = 2,
	weapon_Fireball = 3,
	weapon_MachineGun = 4,
	weapon_MissileLauncher = 5,
	weapon_Bubbler = 7,
	weapon_Blade = 9,
	weapon_SuperMissileLauncher = 10,
	weapon_Nemesis = 12,
	weapon_Spur = 13,
};
