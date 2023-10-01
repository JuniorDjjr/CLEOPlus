
#include "plugin.h"
#include "SpecialModels.h"
#include "CModelInfo.h"
#include "RenderObject.h"

RenderObject::RenderObject(CEntity* _owner, RpClump* _clump, RwFrame* _frame, RpAtomic* _atomic, SpecialModel* _specialModel, int _modelId, int _boneId, float _x, float _y, float _z, float _rx, float _ry, float _rz, float _sx, float _sy, float _sz, float _dx, float _dy, float _dz, float _dw) {
	owner = _owner;
	clump = _clump;
	frame = _frame;
	atomic = _atomic;
	specialModel = _specialModel;
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

RenderObject::~RenderObject() {
	CBaseModelInfo* modelInfo = nullptr;
	if (modelId > 0) {
		(CBaseModelInfo*)CModelInfo::GetModelInfo(modelId);
		if (modelInfo) modelInfo->RemoveRef();
	}
	if (clump) RpClumpDestroy(clump);
	if (atomic) RpAtomicDestroy(atomic);
	if (frame) RwFrameDestroy(frame);
	if (specialModel) {
		if (specialModel->Remove()) {
			delete specialModel;
		}
	}
	owner = nullptr;
}