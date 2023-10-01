#pragma once
#include "OpcodesCommon.h"
#include <set>

class ScriptEvent {
public:
	enum List { SaveConfirmation, CharDelete, CharCreate, CarDelete, CarCreate, ObjectDelete, ObjectCreate, OnMenu, CharProcess, CarProcess, ObjectProcess,
		BuildingProcess, CharDamage, CarWeaponDamage, BulletImpact, BeforeGameProcess, AfterGameProcess, TOTAL_SCRIPT_EVENTS };

	CRunningScript *script;
	int eventScriptIP;
	unsigned int varPointer[4];

	void RunScriptEvent();
	void RunScriptEvent(DWORD arg1);
	void RunScriptEvent(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4);

	static void ClearForScriptLabelAndVar(vector<ScriptEvent*> &scriptEvents, CRunningScript* script, int label, unsigned int varPointer[4]);
	static void ClearAllForScript(CRunningScript *script);
	static void ClearAllScriptEvents();
};

void ScriptDeleteEvent(CScriptThread* script);
void PatchBuildingProcessIfNeeded();
extern vector<ScriptEvent*> scriptEvents[ScriptEvent::TOTAL_SCRIPT_EVENTS];
