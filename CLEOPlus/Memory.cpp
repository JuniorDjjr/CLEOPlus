#include "OpcodesCommon.h"

// 0D4E=4,%4d% = struct %1d% offset %2d% size %3d%
OpcodeResult WINAPI READ_STRUCT_OFFSET(CScriptThread* thread)
{
	unsigned int struc = CLEO_GetIntOpcodeParam(thread);
	unsigned int offset = CLEO_GetIntOpcodeParam(thread);
	unsigned int size = CLEO_GetIntOpcodeParam(thread);
	unsigned int result = 0;

	if (CLEO_GetOperandType(thread) > 8) { // is string
		CLEO_SetIntOpcodeParam(thread, (DWORD)((char *)(struc + offset)));
	}
	else {
		memcpy(&result, (void *)(struc + offset), size);
		CLEO_SetIntOpcodeParam(thread, result);
	}
	return OR_CONTINUE;
}

// 0D4F=4,struct %1d% offset %2d% size %3d% = %4d%
OpcodeResult WINAPI WRITE_STRUCT_OFFSET(CScriptThread* thread)
{
	unsigned int struc = CLEO_GetIntOpcodeParam(thread);
	unsigned int offset = CLEO_GetIntOpcodeParam(thread);
	unsigned int size = CLEO_GetIntOpcodeParam(thread);
	unsigned int value;

	if (CLEO_GetOperandType(thread) > 8) { // is string
		*(char **)(struc + offset) = (char *)CLEO_GetIntOpcodeParam(thread);
	}
	else {
		value = CLEO_GetIntOpcodeParam(thread);
		memcpy((void *)(struc + offset), &value, size);
	}
	return OR_CONTINUE;
}

// 0xD27=3,copy_memory %1d% to %2d% size %3d%
OpcodeResult WINAPI COPY_MEMORY(CScriptThread* thread)
{
	unsigned int from = CLEO_GetIntOpcodeParam(thread);
	unsigned int to = CLEO_GetIntOpcodeParam(thread);
	unsigned int size = CLEO_GetIntOpcodeParam(thread);
	memcpy((void*)to, (void*)from, size);
	return OR_CONTINUE;
}

// 0xE6A=2,make_nop %1d% size %2d%
OpcodeResult WINAPI MAKE_NOP(CScriptThread* thread)
{
	unsigned int address = CLEO_GetIntOpcodeParam(thread);
	unsigned int size = CLEO_GetIntOpcodeParam(thread);
	injector::MakeNOP(address, size, true);
	return OR_CONTINUE;
}

// 0EE2=-1,read_struct_offset_multi %1d% offset %2d% count %3d% size %4d%
OpcodeResult WINAPI READ_STRUCT_OFFSET_MULTI(CScriptThread* thread)
{
	unsigned int struc = CLEO_GetIntOpcodeParam(thread);
	unsigned int offset = CLEO_GetIntOpcodeParam(thread);
	unsigned int count = CLEO_GetIntOpcodeParam(thread);
	unsigned int size = CLEO_GetIntOpcodeParam(thread);

	struc += offset;

	for (unsigned int i = 0; i < (count * size); i += size)
	{
		SCRIPT_VAR *pointer = CLEO_GetPointerToScriptVariable(thread);
		if (pointer > 0) {
			*(DWORD*)pointer = 0;
			memcpy(pointer, (void*)(struc + i), size);
		}
	}
	while(*thread->ip) CLEO_SkipOpcodeParams(thread, 1);
	++thread->ip;
	return OR_CONTINUE;
}

// 0EE3=-1,write_struct_offset_multi %1d% offset %2d% count %3d% size %4d%
OpcodeResult WINAPI WRITE_STRUCT_OFFSET_MULTI(CScriptThread* thread)
{
	unsigned int struc = CLEO_GetIntOpcodeParam(thread);
	unsigned int offset = CLEO_GetIntOpcodeParam(thread);
	unsigned int count = CLEO_GetIntOpcodeParam(thread);
	unsigned int size = CLEO_GetIntOpcodeParam(thread);
	unsigned int value;

	struc += offset;

	for (unsigned int i = 0; i < (count * size); i += size)
	{
		value = CLEO_GetIntOpcodeParam(thread);
		memcpy((void *)(struc + i), &value, size);
	}
	while (*thread->ip) CLEO_SkipOpcodeParams(thread, 1);
	++thread->ip;
	return OR_CONTINUE;
}

// 0D37=3,struct %1d% param %2d% = %3d%
OpcodeResult WINAPI WRITE_STRUCT_PARAM(CScriptThread* thread)
{
	unsigned int *struc = (unsigned int *)CLEO_GetIntOpcodeParam(thread);
	int param = CLEO_GetIntOpcodeParam(thread);
	int value = CLEO_GetIntOpcodeParam(thread);
	memcpy(&struc[param], &value, 4);
	return OR_CONTINUE;
}

// 0D38=3,%3d% = struct %1d% param %2d%
OpcodeResult WINAPI READ_STRUCT_PARAM(CScriptThread* thread)
{
	unsigned int *struc = (unsigned int *)CLEO_GetIntOpcodeParam(thread);
	int param = CLEO_GetIntOpcodeParam(thread);
	int value;
	memcpy(&value, &struc[param], 4);
	CLEO_SetIntOpcodeParam(thread, value);
	return OR_CONTINUE;
}
