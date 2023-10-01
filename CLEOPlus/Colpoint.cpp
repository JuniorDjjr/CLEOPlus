#include "OpcodesCommon.h"
#include "CColpoint.h"
#include "CWorld.h"

OpcodeResult WINAPI GET_COLLISION_BETWEEN_POINTS(CScriptThread* thread)
{
	bool bResult = false;
	CVector origin, target, outPoint; unsigned int flag[8]; CEntity *colEntity, *ignoringEntity; CColPoint *colPoint;

	origin.x = CLEO_GetFloatOpcodeParam(thread);
	origin.y = CLEO_GetFloatOpcodeParam(thread);
	origin.z = CLEO_GetFloatOpcodeParam(thread);
	target.x = CLEO_GetFloatOpcodeParam(thread);
	target.y = CLEO_GetFloatOpcodeParam(thread);
	target.z = CLEO_GetFloatOpcodeParam(thread);
	flag[0] = CLEO_GetIntOpcodeParam(thread);
	flag[1] = CLEO_GetIntOpcodeParam(thread);
	flag[2] = CLEO_GetIntOpcodeParam(thread);
	flag[3] = CLEO_GetIntOpcodeParam(thread);
	flag[4] = CLEO_GetIntOpcodeParam(thread);
	flag[5] = CLEO_GetIntOpcodeParam(thread);
	flag[6] = CLEO_GetIntOpcodeParam(thread);
	flag[7] = CLEO_GetIntOpcodeParam(thread);
	ignoringEntity = (CEntity*)CLEO_GetIntOpcodeParam(thread);
	colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);

	// So isn't required to send the pointer
	bool deleteColPointAfter = false;
	if ((uintptr_t)colPoint <= 0x00A00000) {
		colPoint = new CColPoint();
		deleteColPointAfter = true;
	}

	CWorld::pIgnoreEntity = ignoringEntity;
	bResult = CWorld::ProcessLineOfSight(origin, target, *colPoint, colEntity, flag[0], flag[1], flag[2], flag[3], flag[4], flag[5], flag[6], flag[7]);
	CWorld::pIgnoreEntity = nullptr;

	if (bResult)
	{
		CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecPoint.x);
		CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecPoint.y);
		CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecPoint.z);
		CLEO_SetIntOpcodeParam(thread, (uintptr_t)colEntity);
	}
	else
	{
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetIntOpcodeParam(thread, 0);
	}

	if (deleteColPointAfter) {
		delete colPoint;
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_COLPOINT_NORMAL_VECTOR(CScriptThread* thread)
{
	CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecNormal.x);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecNormal.y);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecNormal.z);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_COLPOINT_DEPTH(CScriptThread* thread)
{
	//CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	//CLEO_SetFloatOpcodeParam(thread, colPoint->m_fDepth);
	uint32_t colPoint = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, *(float*)(colPoint + 0x28));
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_COLPOINT_SURFACE(CScriptThread* thread)
{
	//CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	//CLEO_SetIntOpcodeParam(thread, colPoint->m_nSurfaceTypeB);
	uint32_t colPoint = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, *(uint8_t*)(colPoint+0x23));
	return OR_CONTINUE;
}

// This one ISN'T retrocompatible with 0D3E's NewOpcodes, because DK forgot to separate day and night values.
OpcodeResult WINAPI GET_COLPOINT_LIGHTING(CScriptThread* thread)
{
	uintptr_t colPoint = CLEO_GetIntOpcodeParam(thread);
	bool night = CLEO_GetIntOpcodeParam(thread);

	//uint8_t lightingFull = injector::ReadMemory<uint8_t>(colPoint + 0x25, false);
	uint8_t lightingFull = *(char*)(reinterpret_cast<char*>(colPoint) + 0x25); //lightingB (current plugin-sdk is bugged)

	unsigned int lighting = 0;
	if (night)
	{
		lighting = lightingFull & 0x0F;
	}
	else
	{
		lighting = lightingFull & 0xF0;
	}

	CLEO_SetIntOpcodeParam(thread, lighting);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_COLPOINT_COORDINATES(CScriptThread* thread)
{
	CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecPoint.x);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecPoint.y);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecPoint.z);
	return OR_CONTINUE;
}