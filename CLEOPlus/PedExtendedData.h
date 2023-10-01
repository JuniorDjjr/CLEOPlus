#pragma once
#include "plugin.h"
#include "SpecialModels.h"
#include "RenderObject.h"

using namespace std;
using namespace plugin;

class PedExtended {
public:
	int activeTasks[32];
	CEntity *killTargetPed;
	CEntity *lastDamageEntity;
	int lastDamageWeapon;
	int lastDamagePart;
	float lastDamageIntensity;
	bool ignoreDamageAnims;

	union {
		unsigned int aiFlagsIntValue;
		struct {
			unsigned char bKillingSomething : 1;
			unsigned char bUsingGun : 1;
			unsigned char bFighting : 1;
			unsigned char bFallenOnGround : 1;
			unsigned char bEnteringAnyCar : 1;
			unsigned char bExitingAnyCar : 1;
			unsigned char bPlayingAnyPrimaryScriptAnimation : 1;
			unsigned char bPlayingAnySecondaryScriptAnimation : 1;
			unsigned char bRootTaskIsntImportant : 1;
		} aiFlags;
	};

	vector<RenderObject*> renderObjects;

	PedExtended(CPed *ped) {
		memset(activeTasks, 0, sizeof(activeTasks));
		killTargetPed = nullptr;
		lastDamageEntity = nullptr;
		lastDamageWeapon = 0;
		lastDamagePart = 0;
		lastDamageIntensity = 0.0f;
		ignoreDamageAnims = false;
		aiFlagsIntValue = 0; //reset all flags
		renderObjects.clear();
	}
};

extern PedExtendedData<PedExtended> extData;
