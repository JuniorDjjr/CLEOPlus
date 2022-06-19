#include "OpcodesCommon.h"
#include "CKeyGen.h"

using namespace plugin;
using namespace std;

class ExtendedVars
{
public:
	uint32_t id;
	void *data;

	ExtendedVars(ExtendedVars *)
	{
		id = 0;
		data = 0;
	}

	ExtendedVars(uint32_t newId, uint32_t totalVars)
	{
		id = newId;
		data = malloc(totalVars * 4);
		memset(data, 0, (totalVars * 4));
	}
};

class PedExtVars
{
public:
	list<ExtendedVars*> extendedVarsList;

	PedExtVars(CPed *)
	{
		extendedVarsList.clear();
	}

	~PedExtVars()
	{
		for (ExtendedVars* extendedVars : extendedVarsList)
		{
			free(reinterpret_cast<ExtendedVars*>(extendedVars)->data);
		}
		extendedVarsList.clear();
	}
};

PedExtendedData<PedExtVars> pedExtVars;

class VehExtVars
{
public:
	list<ExtendedVars*> extendedVarsList;

	VehExtVars(CVehicle *)
	{
		extendedVarsList.clear();
	}

	~VehExtVars()
	{
		for (ExtendedVars* extendedVars : extendedVarsList)
		{
			free(reinterpret_cast<ExtendedVars*>(extendedVars)->data);
		}
		extendedVarsList.clear();
	}
};

VehicleExtendedData<VehExtVars> vehExtVars;

class ObjExtVars
{
public:
	list<ExtendedVars*> extendedVarsList;

	ObjExtVars(CObject *)
	{
		extendedVarsList.clear();
	}

	~ObjExtVars()
	{
		for (ExtendedVars* extendedVars : extendedVarsList)
		{
			free(reinterpret_cast<ExtendedVars*>(extendedVars)->data);
		}
		extendedVarsList.clear();
	}
};

ObjectExtendedData<ObjExtVars> objExtVars;


ExtendedVars *FindExtendedVarsFromId(list<ExtendedVars*> extendedVarsList, uint32_t findId)
{
	for (ExtendedVars* extendedVars : extendedVarsList)
	{
		if (reinterpret_cast<ExtendedVars*>(extendedVars)->id == findId) return reinterpret_cast<ExtendedVars*>(extendedVars);
	}
	return nullptr;
}

//////////////////////////////////////////// Ped //////////////////////////////////////////////////

OpcodeResult WINAPI INIT_EXTENDED_CHAR_VARS(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;

	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int totalVars = CLEO_GetIntOpcodeParam(thread);

	PedExtVars &xdata = pedExtVars.Get(ped);
	if (&xdata != nullptr && totalVars > 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			free(extVars->data);
			xdata.extendedVarsList.remove(extVars);
			delete extVars;
		}

		xdata.extendedVarsList.push_back(new ExtendedVars(findId, totalVars));
		bResult = true;
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_EXTENDED_CHAR_VAR(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int varId = CLEO_GetIntOpcodeParam(thread) - 1;
	int varValue = CLEO_GetIntOpcodeParam(thread);

	PedExtVars &xdata = pedExtVars.Get(ped);
	if (&xdata && varId >= 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			*(uint32_t*)(reinterpret_cast<uint32_t>(extVars->data) + (4 * varId)) = varValue;
			bResult = true;
		}
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_EXTENDED_CHAR_VAR(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int varId = CLEO_GetIntOpcodeParam(thread) - 1;
	int varValue = 0;

	PedExtVars &xdata = pedExtVars.Get(ped);
	if (&xdata && varId >= 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			varValue = *(uint32_t*)(reinterpret_cast<uint32_t>(extVars->data) + (4 * varId));
			bResult = true;
		}
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	CLEO_SetIntOpcodeParam(thread, varValue);
	return OR_CONTINUE;
}

//////////////////////////////////////////// Veh //////////////////////////////////////////////////

OpcodeResult WINAPI INIT_EXTENDED_CAR_VARS(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;

	CVehicle *veh = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int totalVars = CLEO_GetIntOpcodeParam(thread);

	VehExtVars &xdata = vehExtVars.Get(veh);
	if (&xdata != nullptr && totalVars > 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			free(extVars->data);
			xdata.extendedVarsList.remove(extVars);
			delete extVars;
		}

		xdata.extendedVarsList.push_back(new ExtendedVars(findId, totalVars));
		bResult = true;
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_EXTENDED_CAR_VAR(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;
	CVehicle *veh = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int varId = CLEO_GetIntOpcodeParam(thread) - 1;
	int varValue = CLEO_GetIntOpcodeParam(thread);

	VehExtVars &xdata = vehExtVars.Get(veh);
	if (&xdata && varId >= 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			*(uint32_t*)(reinterpret_cast<uint32_t>(extVars->data) + (4 * varId)) = varValue;
			bResult = true;
		}
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_EXTENDED_CAR_VAR(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;
	CVehicle *veh = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int varId = CLEO_GetIntOpcodeParam(thread) - 1;
	int varValue = 0;

	VehExtVars &xdata = vehExtVars.Get(veh);
	if (&xdata && varId >= 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			varValue = *(uint32_t*)(reinterpret_cast<uint32_t>(extVars->data) + (4 * varId));
			bResult = true;
		}
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	CLEO_SetIntOpcodeParam(thread, varValue);
	return OR_CONTINUE;
}


//////////////////////////////////////////// Object //////////////////////////////////////////////////

OpcodeResult WINAPI INIT_EXTENDED_OBJECT_VARS(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;

	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int totalVars = CLEO_GetIntOpcodeParam(thread);

	ObjExtVars &xdata = objExtVars.Get(obj);
	if (&xdata != nullptr && totalVars > 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			free(extVars->data);
			xdata.extendedVarsList.remove(extVars);
			delete extVars;
		}

		xdata.extendedVarsList.push_back(new ExtendedVars(findId, totalVars));
		bResult = true;
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_EXTENDED_OBJECT_VAR(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;
	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int varId = CLEO_GetIntOpcodeParam(thread) - 1;
	int varValue = CLEO_GetIntOpcodeParam(thread);

	ObjExtVars &xdata = objExtVars.Get(obj);
	if (&xdata && varId >= 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			*(uint32_t*)(reinterpret_cast<uint32_t>(extVars->data) + (4 * varId)) = varValue;
			bResult = true;
		}
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_EXTENDED_OBJECT_VAR(CScriptThread* thread)
{
	char buf[100];
	bool bResult = false;
	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	uint32_t findId = CKeyGen::GetUppercaseKey(CLEO_ReadStringOpcodeParam(thread, buf, sizeof(buf)));
	int varId = CLEO_GetIntOpcodeParam(thread) - 1;
	int varValue = 0;

	ObjExtVars &xdata = objExtVars.Get(obj);
	if (&xdata && varId >= 0)
	{
		if (findId == 2949658545) // "AUTO"
			findId = (uint32_t)thread;

		ExtendedVars *extVars = FindExtendedVarsFromId(xdata.extendedVarsList, findId);
		if (extVars)
		{
			varValue = *(uint32_t*)(reinterpret_cast<uint32_t>(extVars->data) + (4 * varId));
			bResult = true;
		}
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	CLEO_SetIntOpcodeParam(thread, varValue);
	return OR_CONTINUE;
}

