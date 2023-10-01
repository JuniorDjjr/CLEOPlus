#pragma once
#include "plugin.h"
#include "SpecialModels.h"
#include "RenderObject.h"

using namespace std;
using namespace plugin;

class ObjExtended {
public:
	vector<RenderObject*> renderObjects;

	ObjExtended(CObject *obj) {
		renderObjects.clear();
	}
};

extern ObjectExtendedData<ObjExtended> objExtData;
