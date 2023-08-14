#include "OpcodesCommon.h"
#include "CColpoint.h"
#include "CWorld.h"

// 0D3A=20,get_collision_between_points %1d% %2d% %3d% and %4d% %5d% %6d% flags %7d% %8d% %9d% %10d% %11d% %12d% %13d% %14d% ignore_entity %15d% store_point_to %17d% %18d% %19d% entity_to %20d% colpoint_data_to %16d%
// 0D3A: get_collision_between_points 0@ 1@ 2@ and 3@ 4@ 5@ flags 1 1 1 1 1 1 1 1 ignore_entity 0 store_point_to 0@ 1@ 2@ entity_to 3@ colpoint_data_to STD_COLPOINT_DATA // IF and SET
// SCR: GET_COLLISION_BETWEEN_POINTS
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

// 0D3B=4,get_colpoint_data %1d% normal_XYZ_to %2d% %3d% %4d%
// 0D3B: get_colpoint_data STD_COLPOINT_DATA normal_XYZ_to 0@ 1@ 2@
// SCR: GET_COL_DATA_NORMAL_VECTOR
OpcodeResult WINAPI GET_COLPOINT_NORMAL_VECTOR(CScriptThread* thread)
{
	CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecNormal.x);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecNormal.y);
	CLEO_SetFloatOpcodeParam(thread, colPoint->m_vecNormal.z);
	return OR_CONTINUE;
}

// 0D3C=2,get_colpoint_data %1d% depth_to %2d%
// 0D3C: get_colpoint_data STD_COLPOINT_DATA depth_to 0@
// SCR: GET_COL_DATA_DEPTH
OpcodeResult WINAPI GET_COLPOINT_DEPTH(CScriptThread* thread)
{
	//CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	//CLEO_SetFloatOpcodeParam(thread, colPoint->m_fDepth);
	uint32_t colPoint = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, *(float*)(colPoint + 0x28));
	return OR_CONTINUE;
}

// 0D3D=2,get_colpoint_data %1d% surface_to %2d%
// 0D3D: get_colpoint_data STD_COLPOINT_DATA surface_to 0@
// SCR: GET_COL_DATA_SURFACE
OpcodeResult WINAPI GET_COLPOINT_SURFACE(CScriptThread* thread)
{
	//CColPoint *colPoint = (CColPoint*)CLEO_GetIntOpcodeParam(thread);
	//CLEO_SetIntOpcodeParam(thread, colPoint->m_nSurfaceTypeB);
	uint32_t colPoint = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, *(uint8_t*)(colPoint+0x23));
	return OR_CONTINUE;
}

// This one ISN'T retrocompatible with 0D3E's NewOpcodes, because DK forgot to separate day and night values.
// 0xE6B=2,get_colpoint_lighting %1d% from_night %2d% store_to %3d%
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