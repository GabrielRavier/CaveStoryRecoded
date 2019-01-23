#include <SDL_render.h>
#include <math.h>

#include "flash.h"
#include "render.h"
#include "game.h"
#include "stage.h"


struct FLASH
{
	int x;
	int y;
	flashModes mode;
	int timer;
	int vW;
	int hH;
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
};
FLASH flash =
{
	0,
	0,
	flashModes::none,
	0,
	0,
	0,
	0xFF,
	0xFF,
	0xFE,
	0xFF
};

void setFlash(int x, int y, flashModes mode, int length)
{
	flash.x = x;
	flash.y = y;
	flash.mode = mode;
	flash.timer = length;
	flash.vW = 0;
	flash.hH = 0;
	return;
}

void setFlashColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	flash.r = r;
	flash.g = g;
	flash.b = b;
	flash.a = a;
	return;
}

void flashExplosion()
{
	static bool explosionEnd = false;
	static int explInc = 0;
	static float h = 0;

	if (explosionEnd == true)
	{
		h -= h / 8;
		flash.hH = h;
		if (flash.hH <= 0)
		{
			flash.mode = flashModes::none;
			explInc = 0;
			h = 0;
			explosionEnd = false;
			return;
		}
	}
	else
	{
		flash.timer += 512;
		explInc += flash.timer;

		flash.vW = explInc / 512;
		flash.hH = explInc / 512;

		if (explInc > 655360)
		{
			explosionEnd = true;
			flash.timer = 0;
			explInc = 122880;
			h = gScreenHeight;
		}
	}

	SDL_SetRenderDrawColor(gRenderer, flash.r, flash.g, flash.b, flash.a);
	if (explosionEnd != true)
		drawRect(unitsToPixels(flash.x - pixelsToUnits(flash.vW/2) - gViewport.x), 0, flash.vW, gScreenHeight);
	drawRect(0, unitsToPixels(flash.y - pixelsToUnits(flash.hH/2) - gViewport.y), gScreenWidth, flash.hH);
}

void flashNormal()
{
	--flash.timer;
	if (flash.timer / 2 & 1)
	{
		SDL_SetRenderDrawColor(gRenderer, flash.r, flash.g, flash.b, flash.a);
		SDL_RenderFillRect(gRenderer, nullptr);
	}
	if (flash.timer < 0)
		flash.mode = flashModes::none;
	return;
}

void actFlash()
{
	switch (flash.mode)
	{
	case flashModes::normal:
		flashNormal();
		break;
	case flashModes::explosion:
		flashExplosion();
		break;

	case flashModes::none:
	default:
		break;
	}

	return;
}
