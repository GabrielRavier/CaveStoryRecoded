#pragma once

#include <string>
#include "common.h"

/*

	Yet to be implemented :

    BOA, CIL, CLR, CPS, CRE, CSS, FLA, FOB, INP, NCJ, NUM, SIL, SPS, SSS, STC and XX1

*/


struct TSC
{
	std::string path;
	int size;
	uint8_t *data;
	char mode;
	char flags;
	int p_read;
	int p_write;
	int line;
	int ypos_line[4];
	int wait;
	int wait_next;
	int next_event;
	char select;
	int face;
	int face_x;
	int item;
	int item_y;
	RECT rcText;
	int offsetY;
	uint8_t wait_beam;
};

extern TSC gTsc;

//Functions
bool initTsc();

void loadStageTsc(const std::string& name);
void loadTsc2(const std::string& name);

int startTscEvent(TSC &ptsc, int no);
int jumpTscEvent(TSC &ptsc, int no);
void stopTsc(TSC &ptsc);

int updateTsc(TSC &ptsc = gTsc);
void drawTsc();
