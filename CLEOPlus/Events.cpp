#include "OpcodesCommon.h"
#include "Events.h"
#include "CMenuManager.h"

vector<ScriptEvent*> scriptEvents[ScriptEvent::TOTAL_SCRIPT_EVENTS];
extern bool pausedLastFrame;

void ScriptEvent::ClearAllForScript(CRunningScript* script)
{
	for (int i = 0; i < ScriptEvent::TOTAL_SCRIPT_EVENTS; ++i)
	{
		for (vector<ScriptEvent*>::iterator it = scriptEvents[i].begin(); it != scriptEvents[i].end();)
		{
			ScriptEvent *scriptEvent = (ScriptEvent*)*it;
			if (scriptEvent->script == script)
			{
				it = scriptEvents[i].erase(it);
				delete scriptEvent;
			}
			else ++it;
		}
	}
}

void ScriptEvent::ClearForScriptLabelAndVar(vector<ScriptEvent*> &scriptEvents, CRunningScript* script, int label, unsigned int varPointer[4])
{
	for (vector<ScriptEvent*>::iterator it = scriptEvents.begin(); it != scriptEvents.end();)
	{
		ScriptEvent *scriptEvent = (ScriptEvent*)*it;
		if (scriptEvent->script == script && scriptEvent->eventScriptIP == label)
		{
			bool ok = true;
			for (int i = 0; i < 4; ++i)
			{
				if (scriptEvent->varPointer[i] == 0) break;
				if (scriptEvent->varPointer[i] != varPointer[i])
				{
					ok = false;
					break;
				}
			}
			if (ok)
			{
				it = scriptEvents.erase(it);
				delete scriptEvent;
			}
		}
		else ++it;
	}
}

void ScriptEvent::ClearAllScriptEvents()
{
	for (int i = 0; i < ScriptEvent::TOTAL_SCRIPT_EVENTS; ++i)
	{
		for (auto& scriptEvent : scriptEvents[i]) delete scriptEvent;
	}
	for (int i = 0; i < ScriptEvent::TOTAL_SCRIPT_EVENTS; ++i)
	{
		scriptEvents[i].clear();
	}
}

void ScriptEvent::RunScriptEvent(DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4)
{
	CRunningScript *script = this->script;
	if (!script) return;

	unsigned char *currentIPbackup = script->m_pCurrentIP;

	int ip = this->eventScriptIP;
	if (ip >= 0)
		script->m_pCurrentIP = (unsigned char *)(scriptSpace + ip);
	else
		script->m_pCurrentIP = (script->m_pBaseIP - ip);

	int wakeTimeBackup = this->script->m_nWakeTime;
	script->m_nWakeTime = 0;

	if (this->varPointer[0] > 0x00010000) *(uint32_t*)varPointer[0] = arg1;
	if (this->varPointer[1] > 0x00010000) *(uint32_t*)varPointer[1] = arg2;
	if (this->varPointer[2] > 0x00010000) *(uint32_t*)varPointer[2] = arg3;
	if (this->varPointer[3] > 0x00010000) *(uint32_t*)varPointer[3] = arg4;

	script->Process();

	script->m_pCurrentIP = currentIPbackup;
	script->m_nWakeTime = wakeTimeBackup;
}

void ScriptEvent::RunScriptEvent(DWORD arg)
{
	ScriptEvent::RunScriptEvent(arg, 0, 0, 0);
}

void ScriptDeleteEvent(CScriptThread* script)
{
	ScriptEvent::ClearAllForScript(reinterpret_cast<CRunningScript *>(script));
}

void AddEvent(CScriptThread* thread, vector<ScriptEvent*> &scriptEventList, int args = 1)
{
	int toggle = CLEO_GetIntOpcodeParam(thread);
	int label = CLEO_GetIntOpcodeParam(thread);
	if (toggle)
	{
		ScriptEvent *eventStruct = new ScriptEvent();
		eventStruct->script = reinterpret_cast<CRunningScript*>(thread);
		eventStruct->eventScriptIP = label;

		for (int i = 0; i < 4; ++i)
		{
			uintptr_t pointer = 0;
			if (i < args)
			{
				pointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
			}
			eventStruct->varPointer[i] = pointer;
		}

		scriptEventList.push_back(eventStruct);
	}
	else
	{
		uintptr_t vars[4];
		for (int i = 0; i < 4; ++i)
		{
			uintptr_t pointer = 0;
			if (i < args)
			{
				pointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
			}
			vars[i] = pointer;
		}

		ScriptEvent::ClearForScriptLabelAndVar(scriptEventList, reinterpret_cast<CRunningScript *>(thread), label, vars);
	}
	return;
}

OpcodeResult WINAPI RETURN_SCRIPT_EVENT(CScriptThread* thread)
{
	return OR_INTERRUPT;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_SAVE_CONFIRMATION(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::SaveConfirmation]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_DELETE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharDelete]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_CREATE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharCreate]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_DELETE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarDelete]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_CREATE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarCreate]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_DELETE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::ObjectDelete]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_CREATE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::ObjectCreate]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_ON_MENU(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::OnMenu]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharProcess]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarProcess]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::ObjectProcess]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_BUILDING_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::BuildingProcess]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_DAMAGE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharDamage]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_WEAPON_DAMAGE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarWeaponDamage]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_BULLET_IMPACT(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::BulletImpact], 4);
	return OR_CONTINUE;
}
