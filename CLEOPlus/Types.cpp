#include "OpcodesCommon.h"

OpcodeResult WINAPI GET_VEHICLE_SUBCLASS(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, vehicle->m_nVehicleSubClass);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ENTITY_TYPE(CScriptThread* thread)
{
	CEntity *entity = (CEntity *)CLEO_GetIntOpcodeParam(thread);
	int type;
	switch (entity->m_nType)
	{
	case eEntityType::ENTITY_TYPE_BUILDING:
		type = 1;
		break;
	case eEntityType::ENTITY_TYPE_OBJECT:
		type = 2;
		break;
	case eEntityType::ENTITY_TYPE_PED:
		type = 3;
		break;
	case eEntityType::ENTITY_TYPE_VEHICLE:
		type = 4;
		break;
	default:
		type = 0; //ENTITY_TYPE_OTHER
		break;
	}
	CLEO_SetIntOpcodeParam(thread, type);
	return OR_CONTINUE;
}
