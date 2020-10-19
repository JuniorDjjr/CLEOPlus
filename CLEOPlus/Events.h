#pragma once
#include "OpcodesCommon.h"
#include <set>

class ScriptEvent {
public:
	enum List { SaveConfirmation, CharDelete, CharCreate, CarDelete, CarCreate, ObjectDelete, ObjectCreate, OnMenu, CharProcess, CarProcess, ObjectProcess,
		BuildingProcess, CharDamage, CarWeaponDamage, BulletImpact, TOTAL_SCRIPT_EVENTS };

	CRunningScript *script;
	int eventScriptIP;
	unsigned int varPointer[4];

	void ScriptEvent::RunScriptEvent(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4);
	void ScriptEvent::RunScriptEvent(DWORD arg1);

	static void ClearForScriptLabelAndVar(vector<ScriptEvent*> &scriptEvents, CRunningScript* script, int label, unsigned int varPointer[4]);
	static void ClearAllForScript(CRunningScript *script);
	static void ClearAllScriptEvents();
};

void ScriptDeleteEvent(CScriptThread* script);
extern vector<ScriptEvent*> scriptEvents[ScriptEvent::TOTAL_SCRIPT_EVENTS];
