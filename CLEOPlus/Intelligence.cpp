#include "OpcodesCommon.h"
#include "PedExtendedData.h"

bool CheckPlayingAnyAnim(PedExtended &xdata, int includeAnims) {
	if (includeAnims > 0) { // not NONE
		if (includeAnims == 3) { //BOTH
			if (xdata.aiFlags.bPlayingAnyPrimaryScriptAnimation || xdata.aiFlags.bPlayingAnySecondaryScriptAnimation) {
				return true;
			}
		}
		else {
			if (includeAnims == 1) { //PRIMARY
				if (xdata.aiFlags.bPlayingAnyPrimaryScriptAnimation) {
					return true;
				}
			}
			else {
				if (includeAnims == 2) { //SECONDARY
					if (xdata.aiFlags.bPlayingAnySecondaryScriptAnimation) {
						return true;
					}
				}
			}
		}
	}
	return false;
}


bool TaskIdIsntImportant(int taskId, CPed *ped) {
	if (taskId == TASK_COMPLEX_WANDER ||
		taskId == TASK_SIMPLE_CAR_DRIVE ||
		taskId == TASK_COMPLEX_CAR_DRIVE ||
		taskId == TASK_COMPLEX_CAR_DRIVE_WANDER ||
		taskId == TASK_SIMPLE_NAMED_ANIM || // checked separately
		taskId == TASK_COMPLEX_AVOID_OTHER_PED_WHILE_WANDERING ||
		taskId == TASK_COMPLEX_GANG_LEADER ||
		taskId == TASK_COMPLEX_GANG_FOLLOWER ||
		taskId == TASK_COMPLEX_GOTO_VEHICLE_AND_LEAN ||
		taskId == TASK_COMPLEX_SEEK_ENTITY ||
		taskId == TASK_SIMPLE_PLAYER_ON_FOOT ||
		taskId == TASK_SIMPLE_PLAYER_IN_CAR ||
		taskId == TASK_SIMPLE_STAND_STILL ||
		taskId == TASK_SIMPLE_SCRATCH_HEAD ||
		taskId == TASK_SIMPLE_DO_HAND_SIGNAL ||
		taskId == TASK_COMPLEX_BE_IN_COUPLE ||
		taskId == TASK_SIMPLE_SUNBATHE ||
		taskId == TASK_COMPLEX_SUNBATHE ||
		taskId == TASK_COMPLEX_WALK_ROUND_CAR ||
		taskId == TASK_COMPLEX_WALK_ROUND_FIRE ||
		taskId == TASK_COMPLEX_WALK_ROUND_OBJECT ||
		taskId == TASK_COMPLEX_WALK_ROUND_SHOP ||
		taskId == TASK_COMPLEX_WALK_ROUND_BUILDING_ATTEMPT ||
		taskId == TASK_COMPLEX_PARTNER) {
		return true;
	}
	else {
		if (ped && ped->m_nMoveState <= 4) { // fix for flee task but not really running
			if (taskId == TASK_COMPLEX_FLEE_ANY_MEANS ||
				taskId == TASK_COMPLEX_FLEE_ENTITY ||
				taskId == TASK_COMPLEX_FLEE_POINT ||
				taskId == TASK_COMPLEX_SMART_FLEE_POINT ||
				taskId == TASK_COMPLEX_SMART_FLEE_ENTITY) {
				return true;
			}
		}
	}
	return false;
}

void CacheOnePedTask(CPed *ped, PedExtended &xdata, int &activeTaskIndex, CTask *task, bool isSecondary)
{
	int taskId = task->GetId();
	xdata.activeTasks[activeTaskIndex] = taskId;
	++activeTaskIndex;
	if (activeTaskIndex > 31) activeTaskIndex = 31; // keep adding tasks in the last slot

	int taskOffsetForKillTargetPed = 0;

	switch (taskId) {
	case TASK_COMPLEX_ARREST_PED:
		taskOffsetForKillTargetPed = 4;
		break;
	case TASK_COMPLEX_KILL_PED_ON_FOOT:
		taskOffsetForKillTargetPed = 16;
		break;
	case TASK_SIMPLE_ARREST_PED:
	case TASK_SIMPLE_KILL_PED_WITH_CAR:
	case TASK_SIMPLE_HURT_PED_WITH_CAR:
		taskOffsetForKillTargetPed = 8;
		break;
	case TASK_KILL_PED_FROM_BOAT:
	case TASK_COMPLEX_KILL_CRIMINAL:
	case TASK_COMPLEX_HIT_PED_WITH_CAR:
		taskOffsetForKillTargetPed = 12;
		break;
	case TASK_COMPLEX_FALL_AND_GET_UP:
	case TASK_COMPLEX_FALL_AND_STAY_DOWN:
		xdata.aiFlags.bFallenOnGround = true;
		break;
	case TASK_SIMPLE_USE_GUN:
		xdata.aiFlags.bUsingGun = true;
		break;
	case TASK_SIMPLE_FIGHT:
		xdata.aiFlags.bFighting = true;
		break;
	case TASK_COMPLEX_ENTER_CAR_AS_DRIVER:
	case TASK_COMPLEX_ENTER_CAR_AS_PASSENGER:
		xdata.aiFlags.bEnteringAnyCar = true;
		break;
	case TASK_COMPLEX_LEAVE_ANY_CAR:
	case TASK_COMPLEX_LEAVE_CAR:
	case TASK_COMPLEX_LEAVE_BOAT:
		xdata.aiFlags.bExitingAnyCar = true;
		break;
	case TASK_SIMPLE_NAMED_ANIM:
		if (isSecondary) xdata.aiFlags.bPlayingAnySecondaryScriptAnimation = true; else xdata.aiFlags.bPlayingAnyPrimaryScriptAnimation = true;
		break;
	}

	if (activeTaskIndex == 1) {
		if (!xdata.aiFlags.bKillingSomething) {
			if (taskId == TASK_COMPLEX_BE_IN_GROUP) {
				CTask *subtask = task->GetSubTask();
				if (subtask) {
					if (TaskIdIsntImportant(subtask->GetId(), ped)) {
						xdata.aiFlags.bRootTaskIsntImportant = true;
					}
				}
			}
			else {
				if (TaskIdIsntImportant(taskId, ped)) {
					xdata.aiFlags.bRootTaskIsntImportant = true;
				}
			}
		}
	}

	if (taskOffsetForKillTargetPed > 0) {
		xdata.aiFlags.bKillingSomething = true;
		xdata.killTargetPed = *(CEntity**)(reinterpret_cast<uintptr_t>(task) + taskOffsetForKillTargetPed);
	}
}

// ------------------------------------------------------------------------------------------------

// 0E42=2,is_char_doing_task_id %1d% %2d%
OpcodeResult WINAPI IS_CHAR_DOING_TASK_ID(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &data = extData.Get(ped);
	int taskId = CLEO_GetIntOpcodeParam(thread);

	for (int i = 0; i < 32; ++i) {
		int activeTaskId = data.activeTasks[i];
		if (activeTaskId != -1) {
			if (activeTaskId == taskId) {
				bResult = true;
				break;
			}
		}
		else break;
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

// 0E43=3,get_char_task_pointer_by_id %1d% %2d% store_to %3d%
OpcodeResult WINAPI GET_CHAR_TASK_POINTER_BY_ID(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &data = extData.Get(ped);
	int taskId = CLEO_GetIntOpcodeParam(thread);

	int activeTaskIndex = 0;
	CTask* task;
	if (ped->m_pIntelligence)
	{
		CTaskManager* taskMgr = &ped->m_pIntelligence->m_TaskMgr;
		for (int i = 0; i < 5; i++)
		{
			task = taskMgr->m_aPrimaryTasks[i];
			while (task)
			{
				if (task->GetId() == taskId) {
					bResult = true;
					goto GET_CHAR_TASK_POINTER_BY_ID_jump_end;
				}
				task = task->GetSubTask();
			}
		}

		for (int i = 0; i < 5; i++)
		{
			task = taskMgr->m_aSecondaryTasks[i];
			while (task)
			{
				if (task->GetId() == taskId) {
					bResult = true;
					goto GET_CHAR_TASK_POINTER_BY_ID_jump_end;
				}
				task = task->GetSubTask();
			}
		}
	}

	task = nullptr;

	GET_CHAR_TASK_POINTER_BY_ID_jump_end:
	CLEO_SetIntOpcodeParam(thread, (DWORD)task);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

// 0E44=2,get_char_kill_target_char %1d% store_to %2d%
OpcodeResult WINAPI GET_CHAR_KILL_TARGET_CHAR(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CEntity *entity = nullptr;
	PedExtended &data = extData.Get(ped);
	entity = data.killTargetPed;
	if (reinterpret_cast<int>(entity) == -1 && ped->IsPlayer()) {
		entity = reinterpret_cast<CPlayerPed*>(ped)->m_pPlayerTargettedPed;
	}
	int targetRef = -1;
	if (reinterpret_cast<int>(entity) > 0 && entity->m_nType == eEntityType::ENTITY_TYPE_PED) {
		targetRef = CPools::GetPedRef(reinterpret_cast<CPed*>(entity));
	}
	CLEO_SetIntOpcodeParam(thread, targetRef);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(targetRef > 0);
	return OR_CONTINUE;
}

// 0E46=1,is_char_using_gun %1d%
OpcodeResult WINAPI IS_CHAR_USING_GUN(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &xdata = extData.Get(ped);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(xdata.aiFlags.bUsingGun);
	return OR_CONTINUE;
}

// 0E47=1,is_char_fighting %1d%
OpcodeResult WINAPI IS_CHAR_FIGHTING(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &xdata = extData.Get(ped);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(xdata.aiFlags.bFighting);
	return OR_CONTINUE;
}

// 0E48=1,is_char_fallen_on_ground %1d%
OpcodeResult WINAPI IS_CHAR_FALLEN_ON_GROUND(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &xdata = extData.Get(ped);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(xdata.aiFlags.bFallenOnGround);
	return OR_CONTINUE;
}

// 0E49=1,is_char_entering_any_car %1d%
OpcodeResult WINAPI IS_CHAR_ENTERING_ANY_CAR(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &xdata = extData.Get(ped);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(xdata.aiFlags.bEnteringAnyCar);
	return OR_CONTINUE;
}

// 0E4A=1,is_char_exiting_any_car %1d%
OpcodeResult WINAPI IS_CHAR_EXITING_ANY_CAR(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &xdata = extData.Get(ped);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(xdata.aiFlags.bExitingAnyCar);
	return OR_CONTINUE;
}

// 0E4B=2,is_char_playing_any_script_animation %1d% include_anims %2d%
OpcodeResult WINAPI IS_CHAR_PLAYING_ANY_SCRIPT_ANIMATION(CScriptThread* thread)
{
	bool bResult = true;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	int includeAnims = CLEO_GetIntOpcodeParam(thread);
	PedExtended &xdata = extData.Get(ped);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CheckPlayingAnyAnim(xdata, includeAnims));
	return OR_CONTINUE;
}

// 0E4C=2,is_char_doing_any_important_task %1d% include_anims %2d%
OpcodeResult WINAPI IS_CHAR_DOING_ANY_IMPORTANT_TASK(CScriptThread* thread)
{
	bool bResult = true;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	int includeAnims = CLEO_GetIntOpcodeParam(thread);
	PedExtended &xdata = extData.Get(ped);
	if (xdata.aiFlags.bRootTaskIsntImportant && !xdata.aiFlags.bKillingSomething) {
		bResult = CheckPlayingAnyAnim(xdata, includeAnims);
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

//0E96=1,clear_char_primary_tasks %1d%
OpcodeResult WINAPI CLEAR_CHAR_PRIMARY_TASKS(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	ped->m_pIntelligence->ClearTasks(1, 0);
	return OR_CONTINUE;
}

// 0E97=1,clear_char_secondary_tasks %1d%
OpcodeResult WINAPI CLEAR_CHAR_SECONDARY_TASKS(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	ped->m_pIntelligence->ClearTasks(0, 1);
	return OR_CONTINUE;
}

// 0EFF=3,get_char_simplest_active_task %1d% id_to %2d% pointer_to %3d%
OpcodeResult WINAPI GET_CHAR_SIMPLEST_ACTIVE_TASK(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CTask *task = ped->m_pIntelligence->m_TaskMgr.GetSimplestActiveTask();
	if (task) {
		CLEO_SetIntOpcodeParam(thread, task->GetId());
		CLEO_SetIntOpcodeParam(thread, (DWORD)task);
		reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(true);
	}
	else {
		CLEO_SetIntOpcodeParam(thread, -1);
		CLEO_SetIntOpcodeParam(thread, -1);
		reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(false);
	}
	return OR_CONTINUE;
}
