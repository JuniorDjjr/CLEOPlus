#include "OpcodesCommon.h"

#include "RadarBlip.h"
#include "CPickup.h"
#include "PedExtendedData.h"
#include "VehExtendedData.h"
#include "..\injector\assembly.hpp"

using namespace plugin;
using namespace injector;

extern bool coopOpcodesInstalled;
extern float weatherInterpolationMult;
void PatchCoop();

CTask *__fastcall GetSimplestActiveTask_CarAnimFix(CTaskManager *_this)
{
	CTask *task = _this->GetSimplestActiveTask();
	int id = task->GetId();
	// is playing named anim
	if (id == eTaskType::TASK_SIMPLE_NAMED_ANIM) 
	{
		CTask *task2 = _this->GetSimplestTask(4);
		if (task2)
		{
			id = task2->GetId();
			// if driving, consider it instead and return it
			if (id == eTaskType::TASK_SIMPLE_CAR_DRIVE || id == eTaskType::TASK_SIMPLE_GANG_DRIVEBY)
			{
				return task2;
			}
		}
	}
	return task;
}

void ApplyPatches()
{
	// Fix sitting in car condition to return correctly if is doing named animation inside car
	patch::RedirectCall(0x48A139, GetSimplestActiveTask_CarAnimFix, true);
	patch::RedirectCall(0x48A0C3, GetSimplestActiveTask_CarAnimFix, true);
	patch::RedirectCall(0x5401C1, GetSimplestActiveTask_CarAnimFix, true);
	patch::RedirectCall(0x5401D3, GetSimplestActiveTask_CarAnimFix, true);
	patch::RedirectCall(0x661C56, GetSimplestActiveTask_CarAnimFix, true);
	patch::RedirectCall(0x6AD9E1, GetSimplestActiveTask_CarAnimFix, true);
	patch::RedirectCall(0x6AD9F9, GetSimplestActiveTask_CarAnimFix, true);

	if (!coopOpcodesInstalled) PatchCoop();
	RadarBlip::Patch();
}
