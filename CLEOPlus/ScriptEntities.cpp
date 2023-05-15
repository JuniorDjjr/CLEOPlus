#include "OpcodesCommon.h"

OpcodeResult WINAPI IS_CAR_SCRIPT_CONTROLLED(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	if (vehicle != nullptr && vehicle->m_nCreatedBy == eVehicleCreatedBy::MISSION_VEHICLE) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI MARK_CAR_AS_NEEDED(CScriptThread* thread)
{
	CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread))->m_nCreatedBy = eVehicleCreatedBy::MISSION_VEHICLE;
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CHAR_SCRIPT_CONTROLLED(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	if (ped != nullptr && ped->m_nCreatedBy == 2) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI MARK_CHAR_AS_NEEDED(CScriptThread* thread)
{
	CPools::GetPed(CLEO_GetIntOpcodeParam(thread))->m_nCreatedBy = 2;
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_OBJECT_SCRIPT_CONTROLLED(CScriptThread* thread)
{
	bool bResult = false;
	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	if (obj != nullptr && obj->m_nObjectType == eObjectType::OBJECT_MISSION || obj->m_nObjectType == eObjectType::OBJECT_MISSION2) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI MARK_OBJECT_AS_NEEDED(CScriptThread* thread)
{
	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	if (thread->external || thread->_fC9 != -1)
		obj->m_nObjectType = eObjectType::OBJECT_MISSION2;
	else
		obj->m_nObjectType = eObjectType::OBJECT_MISSION;
	return OR_CONTINUE;
}