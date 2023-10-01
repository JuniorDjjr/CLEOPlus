#include "OpcodesCommon.h"
#include "CUserDisplay.h"

int GetNextAvailableCounterLine()
{
	for (int i = 0; i < 4; ++i)
	{
		if (!CUserDisplay::OnscnTimer.m_aCounters[i].m_nVarId != 0) return i;
	}
	return -1;
}

OpcodeResult WINAPI DISPLAY_ONSCREEN_TIMER_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int direction = CLEO_GetIntOpcodeParam(thread);
	CUserDisplay::OnscnTimer.AddClock(varPointer, 0, direction);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DISPLAY_ONSCREEN_TIMER_WITH_STRING_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int direction = CLEO_GetIntOpcodeParam(thread);
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 8);
	CUserDisplay::OnscnTimer.AddClock(varPointer, string, direction);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DISPLAY_ONSCREEN_COUNTER_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int type = CLEO_GetIntOpcodeParam(thread);
	int lineId = GetNextAvailableCounterLine();
	if (lineId != -1) {
		CUserDisplay::OnscnTimer.AddCounter(varPointer, type, 0, lineId);
	}
	return OR_CONTINUE;
}
 
OpcodeResult WINAPI DISPLAY_ONSCREEN_COUNTER_WITH_STRING_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int type = CLEO_GetIntOpcodeParam(thread);
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 8);
	int lineId = GetNextAvailableCounterLine();
	if (lineId != -1) {
		CUserDisplay::OnscnTimer.AddCounter(varPointer, type, string, lineId);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI DISPLAY_TWO_ONSCREEN_COUNTERS_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	uintptr_t varMaxValuePointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varMaxValuePointer -= (uintptr_t)scriptSpace;
	int lineId = GetNextAvailableCounterLine();
	if (lineId != -1) {
		CUserDisplay::OnscnTimer.AddCounterCounter(varPointer, varMaxValuePointer, 0, lineId);
		CUserDisplay::OnscnTimer.m_aCounters[lineId].m_nColourId = 3; // because AddCounterCounter doesn't initialize the color
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI DISPLAY_TWO_ONSCREEN_COUNTERS_WITH_STRING_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	uintptr_t varMaxValuePointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varMaxValuePointer -= (uintptr_t)scriptSpace;
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 8);
	int lineId = GetNextAvailableCounterLine();
	if (lineId != -1) {
		CUserDisplay::OnscnTimer.AddCounterCounter(varPointer, varMaxValuePointer, string, lineId);
		CUserDisplay::OnscnTimer.m_aCounters[lineId].m_nColourId = 3; // because AddCounterCounter doesn't initialize the color
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI CLEAR_ONSCREEN_TIMER_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	CUserDisplay::OnscnTimer.ClearClock(varPointer);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CLEAR_ONSCREEN_COUNTER_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	CUserDisplay::OnscnTimer.ClearCounter(varPointer);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_ONSCREEN_COUNTER_FLASH_WHEN_FIRST_DISPLAYED_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int flash = CLEO_GetIntOpcodeParam(thread);
	CUserDisplay::OnscnTimer.SetCounterFlashWhenFirstDisplayed(varPointer, flash);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_TIMER_BEEP_COUNTDOWN_TIME_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int secs = CLEO_GetIntOpcodeParam(thread);
	CUserDisplay::OnscnTimer.SetClockBeepCountdownSecs(varPointer, secs);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_ONSCREEN_COUNTER_COLOUR_LOCAL(CScriptThread* thread)
{
	uintptr_t varPointer = reinterpret_cast<uintptr_t>(CLEO_GetPointerToScriptVariable(thread));
	varPointer -= (uintptr_t)scriptSpace;
	int color = CLEO_GetIntOpcodeParam(thread);
	CUserDisplay::OnscnTimer.SetCounterColourID(varPointer, color);
	return OR_CONTINUE;
}
