#include "OpcodesCommon.h"
#include "CWeather.h"
#include "CClock.h"

OpcodeResult WINAPI GET_CURRENT_WEATHER(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, CWeather::OldWeatherType);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_NEXT_WEATHER(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, CWeather::NewWeatherType);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_NEXT_WEATHER(CScriptThread* thread)
{
	CWeather::NewWeatherType = (short)CLEO_GetIntOpcodeParam(thread);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_RAIN_INTENSITY(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, CWeather::Rain);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_RAIN_INTENSITY(CScriptThread* thread)
{
	CWeather::Rain = CLEO_GetFloatOpcodeParam(thread);
	return OR_CONTINUE;
}

// 0E40=1,get_current_hour %1d%
OpcodeResult WINAPI GET_CURRENT_HOUR(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, CClock::ms_nGameClockHours);
	return OR_CONTINUE;
}

// 0E41=1,get_current_minute %1d%
OpcodeResult WINAPI GET_CURRENT_MINUTE(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, CClock::ms_nGameClockMinutes);
	return OR_CONTINUE;
}

// 0xE6C=1,get_day_night_balance %1d%
OpcodeResult WINAPI GET_DAY_NIGHT_BALANCE(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, *(float*)0x8D12C0);
	return OR_CONTINUE;
}

// 0xE6D=1,get_underwaterness %1d%
OpcodeResult WINAPI GET_UNDERWATERNESS(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, CWeather::UnderWaterness);
	return OR_CONTINUE;
}

// 0xE6E=1,get_forced_weather %1d%
OpcodeResult WINAPI GET_FORCED_WEATHER(CScriptThread* thread)
{
	bool bResult = true;
	int weather = CWeather::ForcedWeatherType;
	if (weather == 65535) {
		bResult = false;
		weather = -1;
	}
	CLEO_SetIntOpcodeParam(thread, CWeather::ForcedWeatherType);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}
