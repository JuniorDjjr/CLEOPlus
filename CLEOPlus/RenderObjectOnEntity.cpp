#include "OpcodesCommon.h"
#include "PedExtendedData.h"
#include "ObjExtendedData.h"
#include "CModelInfo.h"
#include "SpecialModels.h"
#include "CTxdStore.h"
#include "RenderObject.h"
#include "Events.h"

RenderObject* CreateRenderObject(CPed* ped, CObject* obj, int model, SpecialModel* specialModel, int boneId, float x, float y, float z, float rx, float ry, float rz)
{
	RenderObject* renderObject = nullptr;
	CEntity* entity = nullptr;
	if (ped) {
		entity = ped;
	}
	else {
		if (obj) {
			entity = obj;
		}
	}

	if (specialModel) {
		renderObject = new RenderObject(entity, 0, 0, 0, specialModel, model, boneId, x, y, z, rx, ry, rz, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		specialModel->refCount += 1;
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

				renderObject = new RenderObject(entity, 0, rwFrame, rpAtomic, nullptr, model, boneId, x, y, z, rx, ry, rz, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			RpClump* clump = (RpClump*)modelInfo->m_pRwObject;
			if (clump) {
				modelInfo->AddRef();

				RpClump* rpClump = (RpClump*)reinterpret_cast<CClumpModelInfo*>(modelInfo)->CreateInstance();

				renderObject = new RenderObject(entity, rpClump, 0, 0, nullptr, model, boneId, x, y, z, rx, ry, rz, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}
	if (renderObject) {
		if (obj) {
			ObjExtended& data = objExtData.Get(obj);
			if (&data != nullptr) {
				data.renderObjects.push_back(renderObject);
			}
		}
		if (ped) {
			PedExtended& data = extData.Get(ped);
			if (&data != nullptr) {
				data.renderObjects.push_back(renderObject);
			}
		}
	}
	return renderObject;
}

void DeleteAllRenderObjectsFromChar(PedExtended &data)
{
	for (RenderObject *renderObject : data.renderObjects)
	{
		delete renderObject;
	}
	data.renderObjects.clear();
}

void DeleteAllRenderObjectsFromObject(ObjExtended& data)
{
	for (RenderObject* renderObject : data.renderObjects)
	{
		delete renderObject;
	}
	data.renderObjects.clear();
}

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
	RenderObject *renderObject = CreateRenderObject(ped, nullptr, model, nullptr, boneId, x, y, z, rx, ry, rz);
	CLEO_SetIntOpcodeParam(thread, (DWORD)renderObject);
	return OR_CONTINUE;
}

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
	RenderObject* renderObject = CreateRenderObject(ped, nullptr, -1, specialModel, boneId, x, y, z, rx, ry, rz);
	CLEO_SetIntOpcodeParam(thread, (DWORD)renderObject);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_OBJECT(CScriptThread* thread)
{
	CObject* obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	int model = CLEO_GetIntOpcodeParam(thread);
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	float rx = CLEO_GetFloatOpcodeParam(thread);
	float ry = CLEO_GetFloatOpcodeParam(thread);
	float rz = CLEO_GetFloatOpcodeParam(thread);
	RenderObject* renderObject = CreateRenderObject(nullptr, obj, model, nullptr, -1, x, y, z, rx, ry, rz);
	CLEO_SetIntOpcodeParam(thread, (DWORD)renderObject);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_OBJECT_FROM_SPECIAL(CScriptThread* thread)
{
	CObject* obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	SpecialModel* specialModel = (SpecialModel*)CLEO_GetIntOpcodeParam(thread);
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	float rx = CLEO_GetFloatOpcodeParam(thread);
	float ry = CLEO_GetFloatOpcodeParam(thread);
	float rz = CLEO_GetFloatOpcodeParam(thread);
	RenderObject* renderObject = CreateRenderObject(nullptr, obj, -1, specialModel, -1, x, y, z, rx, ry, rz);
	CLEO_SetIntOpcodeParam(thread, (DWORD)renderObject);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DELETE_RENDER_OBJECT(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	if (renderObject)
	{
		if (renderObject->owner) {
			eEntityType type = (eEntityType)renderObject->owner->m_nType;

			if (type == eEntityType::ENTITY_TYPE_PED) {

				PedExtended& pedObj = extData.Get((CPed*)renderObject->owner);
				if (&pedObj != nullptr)
				{
					for (std::vector<RenderObject*>::iterator iter = pedObj.renderObjects.begin(); iter != pedObj.renderObjects.end(); ++iter)
					{
						if (*iter == renderObject)
						{
							pedObj.renderObjects.erase(iter);
							break;
						}
					}
				}
			}

			if (type == eEntityType::ENTITY_TYPE_OBJECT) {
				ObjExtended& objData = objExtData.Get((CObject*)renderObject->owner);
				if (&objData != nullptr)
				{
					for (std::vector<RenderObject*>::iterator iter = objData.renderObjects.begin(); iter != objData.renderObjects.end(); ++iter)
					{
						if (*iter == renderObject)
						{
							objData.renderObjects.erase(iter);
							break;
						}
					}
				}
			}
		}
		delete renderObject;
	}
	return OR_CONTINUE;
}

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

OpcodeResult WINAPI SET_RENDER_OBJECT_VISIBLE(CScriptThread* thread)
{
	RenderObject *renderObject = (RenderObject *)CLEO_GetIntOpcodeParam(thread);
	int visibility = CLEO_GetIntOpcodeParam(thread);
	if (renderObject) renderObject->isVisible = visibility;
	return OR_CONTINUE;
}

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
