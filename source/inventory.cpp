#include "game.h"

#include <string>
#include <cstring>
#include <SDL_events.h>
#include <SDL_render.h>
#include "weapons.h"
#include "input.h"
#include "script.h"
#include "sound.h"
#include "render.h"
#include "stage.h"
#include "player.h"
#include "caret.h"
#include "valueview.h"
#include "log.h"

using std::string;
using std::strcpy;


//Inventory
ITEM items[ITEMS];

int inventoryTitleY;
bool inventoryActive;
int selectedItem;

void moveInventoryCursor()
{
	int weaponNo = 0;
	int itemNo = 0;
	while (weaponNo < static_cast<int>(WEAPONS) && weapons[weaponNo].code != 0)
		++weaponNo;
	while (weaponNo < static_cast<int>(ITEMS) && items[itemNo].code != 0)
		++itemNo;

	if (weaponNo || itemNo)
	{
		bool bChange = false;
		if (inventoryActive)
		{
			if (isKeyPressed(keyLeft))
			{
				if (selectedItem % 6)
					--selectedItem;
				else
					selectedItem += 5;

				bChange = true;
			}

			if (isKeyPressed(keyRight))
			{
				if (itemNo - 1 == selectedItem)
					selectedItem = 6 * (selectedItem / 6);
				else if (selectedItem % 6 == 5)
					selectedItem -= 5;
				else
					++selectedItem;

				bChange = true;
			}

			if (isKeyPressed(keyUp))
			{
				if (selectedItem + 5 > 10)
					selectedItem -= 6;
				else
					inventoryActive = false;

				bChange = true;
			}

			if (isKeyPressed(keyDown))
			{
				if (selectedItem / 6 == (itemNo - 1) / 6)
					inventoryActive = false;
				else
					selectedItem += 6;

				bChange = true;
			}

			if (selectedItem >= itemNo)
				selectedItem = itemNo - 1;

			if (inventoryActive && isKeyPressed(keyJump))
				startTscEvent(tsc, items[selectedItem].code + 6000);
		}
		else
		{
			if (isKeyPressed(keyLeft))
			{
				--selectedWeapon;
				bChange = true;
			}
			if (isKeyPressed(keyRight))
			{
				++selectedWeapon;
				bChange = true;
			}
			if (isKeyPressed(keyUp) || isKeyPressed(keyDown))
			{
				if (itemNo)
					inventoryActive = true;
				bChange = true;
			}

			if (selectedWeapon < 0)
				selectedWeapon = weaponNo - 1;

			if (selectedWeapon > weaponNo - 1)
				selectedWeapon = 0;
		}

		if (bChange)
		{
			if (inventoryActive)
			{
				playSound(1);

				if (itemNo)
					startTscEvent(tsc, items[selectedItem].code + 5000);
				else
					startTscEvent(tsc, 5000);
			}
			else
			{
				playSound(4);

				if (weaponNo)
					startTscEvent(tsc, weapons[selectedWeapon].code + 1000);
				else
					startTscEvent(tsc, 1000);
			}
		}
	}
}

void drawInventory()
{
	//Draw main box
	RECT rcBoxTop = { 0, 0, 244, 8 };
	RECT rcBoxBody = { 0, 8, 244, 16 };
	RECT rcBoxBottom = { 0, 16, 244, 24 };

	drawTexture(sprites[TEX_TEXTBOX], &rcBoxTop, screenWidth / 2 - 122, ((screenHeight - 240) / 2) + 8);

	int stripY;
	for (stripY = 1; stripY < 18; ++stripY)
		drawTexture(sprites[TEX_TEXTBOX], &rcBoxBody, screenWidth / 2 - 122, ((screenHeight - 240) / 2) + 8 * (stripY + 1));

	drawTexture(sprites[TEX_TEXTBOX], &rcBoxBottom, screenWidth / 2 - 122, ((screenHeight - 240) / 2) + 8 * (stripY + 1));

	//Draw title text
	RECT rcTitle1 = { 80, 48, 144, 56 };
	RECT rcTitle2 = { 80, 56, 144, 64 };

	if (inventoryTitleY > 16)
		--inventoryTitleY;

	drawTexture(sprites[TEX_TEXTBOX], &rcTitle1, screenWidth / 2 - 112, ((screenHeight - 240) / 2) + inventoryTitleY);
	drawTexture(sprites[TEX_TEXTBOX], &rcTitle2, screenWidth / 2 - 112, ((screenHeight - 240) / 2) + inventoryTitleY + 52);

	//Draw weapon selection box
	RECT rcCur1[2];
	rcCur1[0] = { 0, 88, 40, 128 };
	rcCur1[1] = { 40, 88, 80, 128 };

	static int inventoryFlash;
	++inventoryFlash;
	if (inventoryActive)
		drawTexture(sprites[TEX_TEXTBOX], &rcCur1[1], 40 * selectedWeapon + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 24);
	else
		drawTexture(sprites[TEX_TEXTBOX], &rcCur1[(inventoryFlash >> 1) & 1], 40 * selectedWeapon + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 24);

	//Draw weapons
	RECT rcLv = { 80, 80, 96, 88 };
	RECT rcNone = { 80, 48, 96, 56 };
	RECT rcPer = { 72, 48, 80, 56 };
	RECT rcArms;

	for (size_t i = 0; i < WEAPONS && weapons[i].code; ++i)
	{
		rcArms.left = 16 * (weapons[i].code % 16);
		rcArms.right = rcArms.left + 16;
		rcArms.top = 16 * (weapons[i].code / 16);
		rcArms.bottom = rcArms.top + 16;

		drawTexture(sprites[TEX_ARMSIMAGE], &rcArms, 40 * i + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 24);
		drawTexture(sprites[TEX_TEXTBOX], &rcPer, 40 * i + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 56);
		drawTexture(sprites[TEX_TEXTBOX], &rcLv, 40 * i + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 40);

		drawNumber(weapons[i].level, 40 * i + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 40, false);

		if (weapons[i].max_num)
		{
			drawNumber(weapons[i].num, 40 * i + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 48, false);
			drawNumber(weapons[i].max_num, 40 * i + screenWidth / 2 - 112, ((screenHeight - 240) / 2) + 56, false);
		}
		else
		{
			drawTexture(sprites[TEX_TEXTBOX], &rcNone, 40 * i + screenWidth / 2 - 96, ((screenHeight - 240) / 2) + 48);
			drawTexture(sprites[TEX_TEXTBOX], &rcNone, 40 * i + screenWidth / 2 - 96, ((screenHeight - 240) / 2) + 56);
		}
	}

	//Draw item selection box
	RECT rcCur2[2];

	rcCur2[0] = { 80, 88, 112, 104 };
	rcCur2[1] = { 80, 104, 112, 120 };

	if (inventoryActive)
		drawTexture(
			sprites[TEX_TEXTBOX],
			&rcCur2[(inventoryFlash >> 1) & 1],
			32 * (selectedItem % 6) + screenWidth / 2 - 112,
			16 * (selectedItem / 6) + ((screenHeight - 240) / 2) + 76);
	else
		drawTexture(
			sprites[TEX_TEXTBOX],
			&rcCur2[1],
			32 * (selectedItem % 6) + screenWidth / 2 - 112,
			16 * (selectedItem / 6) + ((screenHeight - 240) / 2) + 76);

	//Draw items
	RECT rcItem;

	for (size_t i = 0; i < ITEMS && items[i].code; ++i)
	{
		rcItem.left = 32 * (items[i].code % 8);
		rcItem.right = rcItem.left + 32;
		rcItem.top = 16 * (items[i].code / 8);
		rcItem.bottom = rcItem.top + 16;

		drawTexture(sprites[TEX_ITEMIMAGE], &rcItem, 32 * (i % 6) + screenWidth / 2 - 112, 16 * (i / 6) + ((screenHeight - 240) / 2) + 76);
	}
}

int openInventory()
{
    logInfo("Started openInventory");

	//Keep track of old one
	string oldScript(tsc.path);

	//Set up some variables
	inventoryTitleY = 24;
	inventoryActive = false;
	selectedItem = 0;

	captureScreen(TEX_SCREENSHOT);

	//Load stage select tsc
	loadTsc2("data/ArmsItem.tsc");

	//Start appropriate event
	size_t weaponNo;
	for (weaponNo = 0; weaponNo < WEAPONS && weapons[weaponNo].code != 0; ++weaponNo);

	if (weaponNo)
		startTscEvent(tsc, weapons[selectedWeapon].code + 1000);
	else
		startTscEvent(tsc, items[selectedItem].code + 5000);

	while (true)
	{
		//Handle events
		getKeys();

		if (isKeyDown(SDL_SCANCODE_ESCAPE))
		{
			const int escape = escapeMenu();

			if (!escape)
				return 0;
			if (escape == 2)
				return 2;
		}

		//Update menu
		if (gameFlags & 2)
			moveInventoryCursor();

		const int tscResult = updateTsc();
		if (!tscResult)
			return 0;
		if (tscResult == 2)
			return 2;

		// Draw screenshot
		drawTextureNoScale(sprites[TEX_SCREENSHOT], nullptr, 0, 0);

		//Draw menu
		drawInventory();
		drawTsc();

		//Check to close
		if (!inventoryActive && (isKeyPressed(keyMenu) || isKeyPressed(keyJump) || isKeyPressed(keyShoot)))
		{
			stopTsc(tsc);
			loadStageTsc(oldScript);
			weaponShiftX = 32;
			return 1;
		}

		if (gameFlags & 2 && (isKeyPressed(keyMenu) || isKeyPressed(keyShoot)))
		{
			stopTsc(tsc);
			loadStageTsc(oldScript);
			weaponShiftX = 32;
			return 1;
		}

		//Present
		if (!drawWindow())
			return 0;
	}
}

