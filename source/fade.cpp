#include <cstring>
#include <algorithm>
#include <SDL_render.h>

#include "render.h"
#include "fade.h"
#include "game.h"

using std::memset;
using std::max;

FADE_STATE fade;

const int fadeFrames = 16;

void initFade() 
{
	memset(&fade, 0, sizeof(FADE_STATE));
}

void updateFade() 
{
	if (fade.mode)
	{
		fade.bMask = false;

		if (++fade.count > (std::max(gScreenWidth, gScreenHeight) / 16 + fadeFrames))
		{
			fade.bMask = (fade.mode == 2);
			fade.mode = 0;
		}
	}
}

void drawFade()
{
	RECT rect = { 0, 0, 0, 16 };

	int xt;
	int yt;
	int frame = 0;

	const int w = gScreenWidth / 16 + 1;
	const int h = gScreenHeight / 16 + 1;
	const int frames = (max(gScreenWidth, gScreenHeight) / 16 + fadeFrames);

	if (fade.bMask)
	{
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 32, 255);
		SDL_RenderClear(gRenderer);
	}
	else if (fade.mode == 1)
	{
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				switch (fade.dir)
				{
				case 0:
					xt = (frames - fade.count);
					frame = xt - x;
					break;

				case 1:
					yt = (frames - fade.count);
					frame = yt - y;
					break;

				case 2:
					xt = w - (frames - fade.count);
					frame = x - xt;
					break;

				case 3:
					yt = h - (frames - fade.count);
					frame = y - yt;
					break;

				case 4:
					frame = fadeFrames - (fade.count - abs(x - (gScreenWidth >> 5)) - abs(y - (gScreenHeight >> 5)));
					break;
				}

				if (frame > 0)
				{
					rect.left = std::min(frame, fadeFrames - 1) * 16;
					rect.right = rect.left + 16;

					const int fixX = (w * 16) - gScreenWidth;
					const int fixY = (h * 16) - gScreenHeight;
					drawTexture(gSprites[TEX_FADE], &rect, x * 16 - fixX, y * 16 - fixY);
				}
			}
		}
	}
	else if (fade.mode == 2)
	{
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				switch (fade.dir)
				{
				case 0:
					xt = w - fade.count;
					frame = x - xt;
					break;

				case 1:
					yt = h - fade.count;
					frame = y - yt;
					break;

				case 2:
					xt = fade.count;
					frame = xt - x;
					break;

				case 3:
					yt = fade.count;
					frame = yt - y;
					break;

				case 4:
					frame = fadeFrames - ((frames - fade.count) - abs(x - (gScreenWidth >> 5)) - abs(y - (gScreenHeight >> 5)));
					break;
				}

				if (frame > 0)
				{
					rect.left = std::min(frame, fadeFrames - 1) * 16;
					rect.right = rect.left + 16;

					const int fixX = (w * 16) - gScreenWidth;
					const int fixY = (h * 16) - gScreenHeight;
					drawTexture(gSprites[TEX_FADE], &rect, x * 16 - fixX, y * 16 - fixY);
				}
			}
		}
	}
}
