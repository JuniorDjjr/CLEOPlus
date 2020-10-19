#pragma once
#include "plugin.h"

using namespace std;
using namespace plugin;

class VehExtended {
public:
	CEntity *lastDamagePed;
	int lastDamageType;
	float lastDamageIntensity;

	VehExtended(CVehicle *vehicle) {
		lastDamagePed = nullptr;
		lastDamageType = 0;
		lastDamageIntensity = 0.0f;
	}
};

extern VehicleExtendedData<VehExtended> vehExtData;
