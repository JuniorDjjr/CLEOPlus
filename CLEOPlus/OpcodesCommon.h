#pragma once
#include "plugin.h"
#include "CLEO.h"
#include "CPickup.h"
#include "CTheScripts.h"

using namespace plugin;
using namespace std;

extern float *ARwidthFromWF;
extern float *ARheightFromWF;
extern int totalZonesToReveal;
extern int currentSaveSlot;
extern int timesGameRestarted;
extern CSprite2d *radarBlipSprites;

extern uintptr_t startPickups;
extern uintptr_t endPickups;
extern uint8_t sizeOfCPickup;
extern uint32_t sizePickups;

extern char *scriptSpace;
extern char bufferA[128];
extern char bufferB[128];
extern char bufferC[128];
extern int gameVersion;
