#include "OpcodesCommon.h"
#include "Events.h"
#include "CMenuManager.h"
#include "RenderObject.h"
#include "ObjExtendedData.h"
#include "..\injector\assembly.hpp"

vector<ScriptEvent*> scriptEvents[ScriptEvent::TOTAL_SCRIPT_EVENTS];
extern bool pausedLastFrame;
extern bool isBuildingProcessPatched;

void ScriptEvent::ClearAllForScript(CRunningScript* script)
{
	for (unsigned int i = 0; i < ScriptEvent::TOTAL_SCRIPT_EVENTS; ++i)
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
			for (unsigned int i = 0; i < 4; ++i)
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
	for (unsigned int i = 0; i < ScriptEvent::TOTAL_SCRIPT_EVENTS; ++i)
	{
		for (auto& scriptEvent : scriptEvents[i]) delete scriptEvent;
	}
	for (unsigned int i = 0; i < ScriptEvent::TOTAL_SCRIPT_EVENTS; ++i)
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

void ScriptEvent::RunScriptEvent()
{
	ScriptEvent::RunScriptEvent(0, 0, 0, 0);
}

void ScriptEvent::RunScriptEvent(DWORD arg)
{
	ScriptEvent::RunScriptEvent(arg, 0, 0, 0);
}

void ScriptDeleteEvent(CScriptThread* script)
{
	ScriptEvent::ClearAllForScript(reinterpret_cast<CRunningScript *>(script));
}

void AddEvent(CScriptThread* thread, vector<ScriptEvent*> &scriptEventList, unsigned int args)
{
	int toggle = CLEO_GetIntOpcodeParam(thread);
	int label = CLEO_GetIntOpcodeParam(thread);
	if (toggle)
	{
		ScriptEvent *eventStruct = new ScriptEvent();
		eventStruct->script = reinterpret_cast<CRunningScript*>(thread);
		eventStruct->eventScriptIP = label;

		for (unsigned int i = 0; i < 4; ++i)
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
		for (unsigned int i = 0; i < 4; ++i)
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

// This is called A LOT, so patch if only if needed. Note: this will also be called when turning off the event, it's fine.
void PatchBuildingProcessIfNeeded()
{
	if (!isBuildingProcessPatched)
	{
		injector::MakeInline<0x535FBE, 0x535FBE + 5>([](injector::reg_pack& regs)
		{
			CEntity* entity = (CEntity*)regs.esi;
			if (entity->m_nType == eEntityType::ENTITY_TYPE_BUILDING)
			{
				if (scriptEvents[ScriptEvent::List::BuildingProcess].size() > 0)
				{
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::BuildingProcess]) scriptEvent->RunScriptEvent((DWORD)entity);
				}
			}
			else if (entity->m_nType == eEntityType::ENTITY_TYPE_OBJECT)
			{
				CObject* object = reinterpret_cast<CObject*>(entity);
				if (scriptEvents[ScriptEvent::List::ObjectProcess].size() > 0)
				{
					int ref = CPools::GetObjectRef(object);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::ObjectProcess]) scriptEvent->RunScriptEvent(ref);
				}
				ObjExtended& xdata = objExtData.Get(object);

			}
			regs.ebx = regs.eax; //mov ebx, eax
			regs.eax = *(uint8_t*)(regs.edi + 0xD); //mov al, [edi+0Dh]
		});
		isBuildingProcessPatched = true;
	}
}

OpcodeResult WINAPI RETURN_SCRIPT_EVENT(CScriptThread* thread)
{
	return OR_INTERRUPT;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_SAVE_CONFIRMATION(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::SaveConfirmation], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_DELETE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharDelete], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_CREATE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharCreate], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_DELETE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarDelete], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_CREATE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarCreate], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_DELETE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::ObjectDelete], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_CREATE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::ObjectCreate], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_ON_MENU(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::OnMenu], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharProcess], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarProcess], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::ObjectProcess], 1);
	PatchBuildingProcessIfNeeded();
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_BUILDING_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::BuildingProcess], 1);
	PatchBuildingProcessIfNeeded();
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_DAMAGE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CharDamage], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_WEAPON_DAMAGE(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::CarWeaponDamage], 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_BULLET_IMPACT(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::BulletImpact], 4);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_BEFORE_GAME_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::BeforeGameProcess], 0);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_EVENT_AFTER_GAME_PROCESS(CScriptThread* thread)
{
	AddEvent(thread, scriptEvents[ScriptEvent::List::AfterGameProcess], 0);
	return OR_CONTINUE;
}
