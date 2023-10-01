#pragma once
#include "plugin.h"
#include "SpecialModels.h"

using namespace std;

class RenderObject {
public:
	CEntity* owner;
	RpClump* clump;
	RwFrame* frame;
	RpAtomic* atomic;
	SpecialModel* specialModel;
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

	RenderObject(CEntity* _owner, RpClump* _clump, RwFrame* _frame, RpAtomic* _atomic, SpecialModel* _specialModel, int _modelId, int _boneId, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _sx, float _sy, float _sz, float _dx, float _dy, float _dz, float _dw);

	~RenderObject();
};
