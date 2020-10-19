#pragma once
#include "plugin.h"

using namespace std;
using namespace plugin;

class RenderObject {
public:
	CEntity *owner;
	RpClump *clump;
	RwFrame *frame;
	RpAtomic *atomic;
	int modelId;
	int boneId;
	CVector offset;
	CVector rot;
	CVector scale;
	RwV4d dist;

	bool isVisible;
	bool hideIfDead;
	bool hideIfWeapon;
	bool hideIfCar;

	RenderObject(CEntity *_owner, RpClump *_clump, RwFrame *_frame, RpAtomic *_atomic, int _modelId, int _boneId, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _sx, float _sy, float _sz, float _dx, float _dy, float _dz, float _dw) {
		owner = _owner;
		clump = _clump;
		frame = _frame;
		atomic = _atomic;
		modelId = _modelId;
		boneId = _boneId;
		offset.x = _x;
		offset.y = _y;
		offset.z = _z;
		rot.x = _rx;
		rot.y = _ry;
		rot.z = _rz;
		scale.x = _sx;
		scale.y = _sy;
		scale.z = _sz;
		dist.x = _dx;
		dist.y = _dy;
		dist.z = _dz;
		dist.w = _dw;
		isVisible = true;
		hideIfDead = false;
		hideIfWeapon = false;
		hideIfCar = false;
	}
};

class PedExtended {
public:
	CTask *activeTasks[32];
	CTask *simplestActiveTask;
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
		simplestActiveTask = nullptr;
		killTargetPed = nullptr;
		lastDamageEntity = nullptr;
		lastDamageWeapon = 0;
		lastDamagePart = 0;
		lastDamageIntensity = 0.0f;
		aiFlagsIntValue = 0; //reset all flags
		renderObjects.clear();
	}
};

extern PedExtendedData<PedExtended> extData;
