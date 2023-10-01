#include "OpcodesCommon.h"

OpcodeResult WINAPI GET_AUDIO_SFX_VOLUME(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, *(float*)0xB5FCCC);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_AUDIO_RADIO_VOLUME(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, *(float*)0xB5FCC8);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_AUDIOSTREAM_INTERNAL(CScriptThread* thread)
{
	HSTREAM stream = CLEO_GetInternalAudioStream(thread, CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, stream);
	return OR_CONTINUE;
}
