#include "OpcodesCommon.h"
#include "CPickups.h"

OpcodeResult WINAPI GET_PICKUP_THIS_COORD(CScriptThread* thread)
{
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	bool onlyAvailable = CLEO_GetIntOpcodeParam(thread);

	CPickup *pickup = nullptr;
	int pickupRefResult = 0;
	bool result = false;

	CVector coord = {x, y, z};

	for (unsigned int i = 0; i <= sizePickups; ++i)
	{
		CPickup *pickup = (CPickup *)(startPickups + (i * sizeOfCPickup));
		CVector pickupCoord = pickup->GetPosn();

		if (DistanceBetweenPoints(pickupCoord, coord) < 0.5f)
		{
			int pickupRef = i | (pickup->m_nReferenceIndex << 16);
			if (!onlyAvailable || !CPickups::IsPickUpPickedUp(pickupRef))
			{
				pickupRefResult = pickupRef;
				result = true;
				break;
			}
		}

	}
	CLEO_SetIntOpcodeParam(thread, pickupRefResult);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_PICKUP_MODEL(CScriptThread* thread)
{
	int pickupIndex = CLEO_GetIntOpcodeParam(thread);
	int actualIndex = CPickups::GetActualPickupIndex(pickupIndex);
	int model = -1;
	bool result = false;
	if (actualIndex != -1)
	{
		CPickup *pickup = (CPickup *)(startPickups + (actualIndex * sizeOfCPickup));
		model = pickup->m_nModelIndex;
		result = true;
	}
	CLEO_SetIntOpcodeParam(thread, model);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_PICKUP_POINTER(CScriptThread* thread)
{
	int pickupIndex = CLEO_GetIntOpcodeParam(thread);
	int actualIndex = CPickups::GetActualPickupIndex(pickupIndex);
	CPickup *pickup = nullptr;
	bool result = false;
	if (actualIndex != -1)
	{
		pickup = (CPickup *)(startPickups + (actualIndex * sizeOfCPickup));
		result = true;
	}
	CLEO_SetIntOpcodeParam(thread, (DWORD)pickup);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_PICKUP_TYPE(CScriptThread* thread)
{
	int pickupIndex = CLEO_GetIntOpcodeParam(thread);
	int actualIndex = CPickups::GetActualPickupIndex(pickupIndex);
	int type = -1;
	bool result = false;
	if (actualIndex != -1)
	{
		CPickup *pickup = (CPickup *)(startPickups + (actualIndex * sizeOfCPickup));
		type = pickup->m_nPickupType;
		result = true;
	}
	CLEO_SetIntOpcodeParam(thread, (DWORD)type);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(result);
	return OR_CONTINUE;
}
