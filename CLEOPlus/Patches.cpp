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

	// Fix IS_CHAR_DEAD returning false even if health is 0.0 (wtf? why? this caused my script and Bullet Physics Ragdoll to not work correctly)
	struct IsCharDeadFix
	{
		void operator()(reg_pack& regs)
		{
			CPed* ped = (CPed*)(regs.eax);
			if (ped->m_fHealth <= 0.0f && ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_nType != eWeaponType::WEAPON_PARACHUTE) {
				regs.eax = ePedState::PEDSTATE_DEAD;
			}
			else {
				regs.eax = ped->m_nPedState; //original code
			}
		}
	}; 
	if (GetGameVersion() == GAME_10US_HOODLUM) {
		MakeInline<IsCharDeadFix>(0x156E7B4, 0x156E7B4 + 6);
	}
	else {
		MakeInline<IsCharDeadFix>(0x464D74, 0x464D74 + 6);
	}

	if (!coopOpcodesInstalled) PatchCoop();
	RadarBlip::Patch();
}
