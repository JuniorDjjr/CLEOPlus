#include "OpcodesCommon.h"
#include "PedExtendedData.h"
#include "CModelInfo.h"
#include "SpecialModels.h"

RenderObject* CreateRenderObject(CPed* ped, int model, SpecialModel* specialModel, int boneId, float x, float y, float z, float rx, float ry, float rz)
{
	RenderObject* renderObject = nullptr;

	if (specialModel) {
		renderObject = new RenderObject(ped, specialModel->clump, 0, 0, model, boneId, x, y, z, rx, ry, rz, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	else {
		CBaseModelInfo* modelInfo = (CBaseModelInfo*)CModelInfo::GetModelInfo(model);
		int rwModelType = modelInfo->GetRwModelType();
		if (rwModelType == 1) {
			RpAtomic* atomic = (RpAtomic*)modelInfo->m_pRwObject;
			if (atomic) {
				modelInfo->AddRef();

				RwFrame* rwFrame = RwFrameCreate();
				RpAtomic* rpAtomic = RpAtomicClone(atomic);
				RpAtomicSetFrame(rpAtomic, rwFrame);

				renderObject = new RenderObject(ped, 0, rwFrame, rpAtomic, model, boneId, x, y, z, rx, ry, rz, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			RpClump* clump = (RpClump*)modelInfo->m_pRwObject;
			if (clump) {
				modelInfo->AddRef();

				RpClump* rpClump = (RpClump*)reinterpret_cast<CClumpModelInfo*>(modelInfo)->CreateInstance();

				renderObject = new RenderObject(ped, rpClump, 0, 0, model, boneId, x, y, z, rx, ry, rz, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}
	if (renderObject) {
		PedExtended& data = extData.Get(ped);
		if (&data != nullptr) {
			data.renderObjects.push_back(renderObject);
		}
	}
	return renderObject;
}

void DeleteAllRenderObjectsFromChar(PedExtended &data)
{
	for (RenderObject *renderObject : data.renderObjects)
	{
		CBaseModelInfo *modelInfo = (CBaseModelInfo *)CModelInfo::GetModelInfo(renderObject->modelId);
		if (modelInfo) modelInfo->RemoveRef();
		if (renderObject->clump) RpClumpDestroy(renderObject->clump);
		if (renderObject->atomic) RpAtomicDestroy(renderObject->atomic);
		if (renderObject->frame) RwFrameDestroy(renderObject->frame);
		renderObject->owner = nullptr;
		delete renderObject;
	}
	data.renderObjects.clear();
}

//CREATE_RENDER_OBJECT_TO_CHAR_BONE scplayer 1000 5 0.0 0.0 0.0 0.0 0.0 0.0 (renderObject)
// 0E2E=10,create_render_object_to_char_bone %1d% model %2d% bone %3d% offset %4d% %5d% %6d% rotation %7d% %8d% %9d% scale %10d% %11d% %12d% store_to %13d%
OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_CHAR_BONE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	int model = CLEO_GetIntOpcodeParam(thread);
	int boneId = CLEO_GetIntOpcodeParam(thread);
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	float rx = CLEO_GetFloatOpcodeParam(thread);
	float ry = CLEO_GetFloatOpcodeParam(thread);
	float rz = CLEO_GetFloatOpcodeParam(thread);
	RenderObject *renderObject = CreateRenderObject(ped, model, nullptr, boneId, x, y, z, rx, ry, rz);
	CLEO_SetIntOpcodeParam(thread, (DWORD)renderObject);
	return OR_CONTINUE;
}

//CREATE_RENDER_OBJECT_TO_CHAR_BONE_FROM_SPECIAL scplayer hSpecialModel 5 0.0 0.0 0.0 0.0 0.0 0.0 (renderObject)
// 0F02=10,create_render_object_to_char_bone_from_special %1d% special_model %2d% bone %3d% offset %4d% %5d% %6d% rotation %7d% %8d% %9d% scale %10d% %11d% %12d% store_to %13d%
OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_CHAR_BONE_FROM_SPECIAL(CScriptThread* thread)
{
	CPed* ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	SpecialModel* specialModel = (SpecialModel*)CLEO_GetIntOpcodeParam(thread);
	int boneId = CLEO_GetIntOpcodeParam(thread);
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	float rx = CLEO_GetFloatOpcodeParam(thread);
	float ry = CLEO_GetFloatOpcodeParam(thread);
	float rz = CLEO_GetFloatOpcodeParam(thread);
	RenderObject* renderObject = CreateRenderObject(ped, -1, specialModel, boneId, x, y, z, rx, ry, rz);
	CLEO_SetIntOpcodeParam(thread, (DWORD)renderObject);
	return OR_CONTINUE;
}

//DELETE_RENDER_OBJECT renderObject
// 0E2F=1,delete_render_object %1d%
OpcodeResult WINAPI DELETE_RENDER_OBJECT(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject)
	{
		PedExtended &data = extData.Get((CPed*)renderObject->owner);
		if (&data != nullptr)
		{
			for (std::vector<RenderObject*>::iterator iter = data.renderObjects.begin(); iter != data.renderObjects.end(); ++iter)
			{
				if (*iter == renderObject)
				{
					data.renderObjects.erase(iter);
					break;
				}
			}
		}
		CBaseModelInfo *modelInfo = (CBaseModelInfo *)CModelInfo::GetModelInfo(renderObject->modelId);
		if (modelInfo) modelInfo->RemoveRef();
		if (renderObject->clump) RpClumpDestroy(renderObject->clump);
		if (renderObject->atomic) RpAtomicDestroy(renderObject->atomic);
		if (renderObject->frame) RwFrameDestroy(renderObject->frame);
		delete renderObject;
	}
	return OR_CONTINUE;
}

//SET_RENDER_OBJECT_AUTO_HIDE renderObject FALSE
// 0E30=4,set_render_object_auto_hide %1d% dead %2d% weapon %3d% car %4d%
OpcodeResult WINAPI SET_RENDER_OBJECT_AUTO_HIDE(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject) {
		renderObject->hideIfDead = CLEO_GetIntOpcodeParam(thread);
		renderObject->hideIfWeapon = CLEO_GetIntOpcodeParam(thread);
		renderObject->hideIfCar = CLEO_GetIntOpcodeParam(thread);
	}
	return OR_CONTINUE;
}

//SET_RENDER_OBJECT_VISIBLE renderObject FALSE
// 0E31=2,set_render_object_visible %1d% %2d%
OpcodeResult WINAPI SET_RENDER_OBJECT_VISIBLE(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	int visibility = CLEO_GetIntOpcodeParam(thread);
	if (renderObject) renderObject->isVisible = visibility;
	return OR_CONTINUE;
}

//SET_RENDER_OBJECT_POSITION renderObject 0.0 0.0 0.0
// 0E35=4,set_render_object_position %1d% %2d% %3d% %4d%
OpcodeResult WINAPI SET_RENDER_OBJECT_POSITION(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject) {
		renderObject->offset.x = CLEO_GetFloatOpcodeParam(thread);
		renderObject->offset.y = CLEO_GetFloatOpcodeParam(thread);
		renderObject->offset.z = CLEO_GetFloatOpcodeParam(thread);
	}
	return OR_CONTINUE;
}

//SET_RENDER_OBJECT_ROTATION renderObject 0.0 0.0 0.0
// 0E36=4,set_render_object_rotation %1d% %2d% %3d% %4d%
OpcodeResult WINAPI SET_RENDER_OBJECT_ROTATION(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject) {
		renderObject->rot.x = CLEO_GetFloatOpcodeParam(thread);
		renderObject->rot.y = CLEO_GetFloatOpcodeParam(thread);
		renderObject->rot.z = CLEO_GetFloatOpcodeParam(thread);
	}
	return OR_CONTINUE;
}

//SET_RENDER_OBJECT_SCALE renderObject 0.0 0.0 0.0
// 0E37=4,set_render_object_scale %1d% %2d% %3d% %4d%
OpcodeResult WINAPI SET_RENDER_OBJECT_SCALE(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject) {
		renderObject->scale.x = CLEO_GetFloatOpcodeParam(thread);
		renderObject->scale.y = CLEO_GetFloatOpcodeParam(thread);
		renderObject->scale.z = CLEO_GetFloatOpcodeParam(thread);
	}
	return OR_CONTINUE;
}

//SET_RENDER_OBJECT_DISTORTION renderObject 0.0 0.0 0.0 0.0
// 0E37=5,set_render_object_distortion %1d% %2d% %3d% %4d% %5d%
OpcodeResult WINAPI SET_RENDER_OBJECT_DISTORTION(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject) {
		renderObject->dist.x = CLEO_GetFloatOpcodeParam(thread);
		renderObject->dist.y = CLEO_GetFloatOpcodeParam(thread);
		renderObject->dist.z = CLEO_GetFloatOpcodeParam(thread);
		renderObject->dist.w = CLEO_GetFloatOpcodeParam(thread);
	}
	return OR_CONTINUE;
}
