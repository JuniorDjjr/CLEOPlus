#include "OpcodesCommon.h"

// 0E21=1,get_audio_sfx_volume %1d%
OpcodeResult WINAPI GET_AUDIO_SFX_VOLUME(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, *(float*)0xB5FCCC);
	return OR_CONTINUE;
}

// 0E22=1,get_audio_radio_volume %1d%
OpcodeResult WINAPI GET_AUDIO_RADIO_VOLUME(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, *(float*)0xB5FCC8);
	return OR_CONTINUE;
}

//// 0E3B=2,get_audiostream_internal %1d% store_to %2d%
OpcodeResult WINAPI GET_AUDIOSTREAM_INTERNAL(CScriptThread* thread)
{
	HSTREAM stream = CLEO_GetInternalAudioStream(thread, CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, stream);
	return OR_CONTINUE;
}
