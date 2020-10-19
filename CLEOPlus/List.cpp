#include "OpcodesCommon.h"

using namespace std;

class ScriptList {
public:
	int type;
	uintptr_t listPointer;
};

vector<ScriptList*> scriptListList;

void ClearScriptLists() {
	for (auto& scriptList : scriptListList) {
		delete scriptList;
	}
	scriptListList.clear();
}

OpcodeResult WINAPI CREATE_LIST(CScriptThread* thread)
{
	int type = CLEO_GetIntOpcodeParam(thread);
	ScriptList *scriptList = new ScriptList();
	scriptList->type = type;
	if (type == 0) // int
	{
		list<int> *l = new list<int>();
		scriptList->listPointer = (DWORD)l;
	}
	else if (type == 1) // float
	{
		list<float> *l = new list<float>();
		scriptList->listPointer = (DWORD)l;
	}
	else if (type == 2) // string
	{
		list<string> *l = new list<string>();
		scriptList->listPointer = (DWORD)l;
	}
	scriptListList.push_back(scriptList);
	CLEO_SetIntOpcodeParam(thread, (DWORD)scriptList);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DELETE_LIST(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	// prefer safer codes for delete
	if (scriptList)
	{
		if (scriptList->type == 0) // int
		{
			list<int> *l = (list<int>*)scriptList->listPointer;
			if (l)
			{
				l->clear(); // useful?
				delete l;
			}
		}
		else if (scriptList->type == 1) // float
		{
			list<float> *l = (list<float>*)scriptList->listPointer;
			if (l)
			{
				l->clear();
				delete l;
			}
		}
		else if (scriptList->type == 2) // string
		{
			list<string> *l = (list<string>*)scriptList->listPointer;
			if (l)
			{
				l->clear();
				delete l;
			}
		}
		delete scriptList;
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_ADD(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		int value = CLEO_GetIntOpcodeParam(thread);
		list<int> *l = (list<int>*)scriptList->listPointer;
		l->push_back(value);
	}
	else if(scriptList->type == 1) // float 
	{
		float value = CLEO_GetFloatOpcodeParam(thread);
		list<float> *l = (list<float>*)scriptList->listPointer;
		l->push_back(value);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_ADD_STRING(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if(scriptList->type == 2) // string
	{
		string value = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
		list<string> *l = (list<string>*)scriptList->listPointer;
		l->push_back(value);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_REMOVE_VALUE(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		int value = CLEO_GetIntOpcodeParam(thread);
		list<int> *l = (list<int>*)scriptList->listPointer;
		l->remove(value);
	}
	else if (scriptList->type == 1) // float
	{
		float value = CLEO_GetFloatOpcodeParam(thread);
		list<float> *l = (list<float>*)scriptList->listPointer;
		l->remove(value);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_REMOVE_STRING_VALUE(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 2) // string
	{
		string value = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
		list<string> *l = (list<string>*)scriptList->listPointer;
		l->remove(value);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_REMOVE_INDEX(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		list<int>::iterator it;
		it = l->begin();
		advance(it, index);
		l->erase(it);
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		list<float>::iterator it;
		it = l->begin();
		advance(it, index);
		l->erase(it);
	}
	else if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		list<string>::iterator it;
		it = l->begin();
		advance(it, index);
		l->erase(it);
	}
	return OR_CONTINUE; 
}

OpcodeResult WINAPI LIST_REMOVE_INDEX_RANGE(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	int index = CLEO_GetIntOpcodeParam(thread);
	int indexEnd = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		list<int>::iterator it, itEnd;
		it = l->begin();
		advance(it, index);
		itEnd = l->begin();
		advance(itEnd, indexEnd);
		l->erase(it, itEnd);
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		list<float>::iterator it, itEnd;
		it = l->begin();
		advance(it, index);
		itEnd = l->begin();
		advance(itEnd, indexEnd);
		l->erase(it, itEnd);
	}
	else if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		list<string>::iterator it, itEnd;
		it = l->begin();
		advance(it, index);
		itEnd = l->begin();
		advance(itEnd, indexEnd);
		l->erase(it, itEnd);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LIST_SIZE(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	int size = 0;
	if (scriptList)
	{
		if (scriptList->type == 0) // int
		{
			list<int> *l = (list<int>*)scriptList->listPointer;
			size = l->size();
		}
		else if (scriptList->type == 1) // float
		{
			list<float> *l = (list<float>*)scriptList->listPointer;
			size = l->size();
		}
		else if (scriptList->type == 2) // string
		{
			list<string> *l = (list<string>*)scriptList->listPointer;
			size = l->size();
		}
	}
	CLEO_SetIntOpcodeParam(thread, size);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LIST_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		list<int>::iterator it;
		it = l->begin();
		advance(it, index);
		CLEO_SetIntOpcodeParam(thread, *it);
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		list<float>::iterator it;
		it = l->begin();
		advance(it, index);
		CLEO_SetFloatOpcodeParam(thread, *it);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		list<string>::iterator it;
		it = l->begin();
		advance(it, index);
		string str = *it;
		CLEO_WriteStringOpcodeParam(thread, &str[0]);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI RESET_LIST(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		l->clear();
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		l->clear();
	}
	else if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		l->clear();
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI REVERSE_LIST(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		l->reverse();
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		l->reverse();
	}
	else if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		l->reverse();
	}
	return OR_CONTINUE;
}
