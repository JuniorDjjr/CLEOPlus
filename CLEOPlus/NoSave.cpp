#include "OpcodesCommon.h"

#include "CModelInfo.h"
#include "CWorld.h"
#include "CPool.h"
#include "CTheCarGenerators.h"
#include "CCarGenerator.h"
#include "game_sa/CarGeneratorsEx.h"

OpcodeResult WINAPI SET_CAR_GENERATOR_NO_SAVE(CScriptThread* thread)
{
	int ref = CLEO_GetIntOpcodeParam(thread);

	int8_t size = injector::ReadMemory<int8_t>(0x006F32AA, true);
	if (size >= 0x30)
	{
		CCarGenerator_extended * generator = reinterpret_cast<CCarGenerator_extended*>(&CTheCarGenerators::CarGeneratorArray[ref]);
		generator->exIplFile = 0xFFFF;
	}
	else {
		CCarGenerator * generator = reinterpret_cast<CCarGenerator_extended*>(&CTheCarGenerators::CarGeneratorArray[ref]);
		generator->m_nIplId = (char)0xFF;
	}

	return OR_CONTINUE;
}

OpcodeResult WINAPI CREATE_OBJECT_NO_SAVE(CScriptThread* thread)
{
	int mi = CLEO_GetIntOpcodeParam(thread);
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	int offset = CLEO_GetIntOpcodeParam(thread);
	int fixGround = CLEO_GetIntOpcodeParam(thread);

	if (mi < 0) mi = CTheScripts::UsedObjectArray[-mi].dwModelIndex; // not tested

	CBaseModelInfo * pModel = CModelInfo::GetModelInfo(mi);
	pModel->m_nAlpha = -1;

	CObject * object = CObject::Create(mi);

	object->m_nObjectType = eObjectType::OBJECT_MISSION2;

	if (z <= -100.0) {
		z = CWorld::FindGroundZForCoord(x, y);
	}
	else if (fixGround) {
		bool foundGround;
		CEntity *foundEntity;
		float foundZ = CWorld::FindGroundZFor3DCoord(x, y, z, &foundGround, &foundEntity);
		if (foundGround) {
			z = foundZ;
		}
	}
	if (offset) {
		z = object->GetDistanceFromCentreOfMassToBaseOfModel() + z;
	}

	object->SetPosn(x, y, z);
	object->SetOrientation(0.0, 0.0, 0.0);

	plugin::CallMethod<0x446F90, CEntity *>(object); // Update RwMatrix
	object->UpdateRwFrame();

	if (pModel->AsLodAtomicModelInfoPtr())
		object->SetupBigBuilding();

	CVector coord = { x,y,z };
	CTheScripts::ClearSpaceForMissionEntity(coord, object);

	CWorld::Add(object);

	RpAtomic *atomic = (RpAtomic *)object->m_pRwObject;

	CLEO_SetIntOpcodeParam(thread, CPools::GetObjectRef(object));

	return OR_CONTINUE;
}