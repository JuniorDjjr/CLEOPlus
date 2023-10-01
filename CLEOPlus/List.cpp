#include "OpcodesCommon.h"

using namespace std;

class ScriptList {
public:
	int type;
	uintptr_t listPointer;
	// test bug fix
	list<int>* intList;
	list<float>* floatList;
	list<string>* stringList;

	ScriptList(int _type)
	{
		type = _type;
		listPointer = 0;
		intList = nullptr;
		floatList = nullptr;
		stringList = nullptr;
		if (_type == 0) // int
		{
			intList = new list<int>();
			listPointer = (DWORD)intList;
		}
		else if (_type == 1) // float
		{
			floatList = new list<float>();
			listPointer = (DWORD)floatList;
		}
		else if (_type == 2) // string
		{
			stringList = new list<string>();
			listPointer = (DWORD)stringList;
		}
	}

	~ScriptList() {
		if (listPointer != 0)
		{
			if (type == 0) // int
			{
				list<int>* l = (list<int>*)listPointer;
				l->clear(); // useful?
				delete l;
			}
			else if (type == 1) // float
			{
				list<float>* l = (list<float>*)listPointer;
				l->clear();
				delete l;
			}
			else if (type == 2) // string
			{
				list<string>* l = (list<string>*)listPointer;
				l->clear();
				delete l;
			}
			listPointer = 0;
		}
	}
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
	ScriptList *scriptList = new ScriptList(type);
	if (scriptList->listPointer == 0) {
		MessageBox(HWND_DESKTOP, string("CREATE_LIST failed: cant allocate new list " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
	}
	scriptListList.push_back(scriptList);
	CLEO_SetIntOpcodeParam(thread, (DWORD)scriptList);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DELETE_LIST(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList)
	{
		std::vector<ScriptList*>::iterator position = std::find(scriptListList.begin(), scriptListList.end(), scriptList);
		if (position != scriptListList.end()) scriptListList.erase(position);

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
	else if (scriptList->type == 1) // float 
	{
		float value = CLEO_GetFloatOpcodeParam(thread);
		list<float> *l = (list<float>*)scriptList->listPointer;
		l->push_back(value);
	}
	else {
		MessageBox(HWND_DESKTOP, string("LIST_ADD failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		int value = CLEO_GetIntOpcodeParam(thread); // fallback
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_ADD_STRING(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	string value = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	if(scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		l->push_back(value);
	}
	else {
		MessageBox(HWND_DESKTOP, string("LIST_ADD_STRING failed: list type wrong " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
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
	else {
		MessageBox(HWND_DESKTOP, string("LIST_REMOVE_VALUE failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		int value = CLEO_GetIntOpcodeParam(thread); //fallback
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_REMOVE_STRING_VALUE(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	string value = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		l->remove(value);
	}
	else {
		MessageBox(HWND_DESKTOP, string("LIST_REMOVE_STRING_VALUE failed: list type wrong " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LIST_REMOVE_INDEX(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		if (index < l->size()) {
			list<int>::iterator it;
			it = l->begin();
			advance(it, index);
			l->erase(it);
		}
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		if (index < l->size()) {
			list<float>::iterator it;
			it = l->begin();
			advance(it, index);
			l->erase(it);
		}
	}
	else if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		if (index < l->size()) {
			list<string>::iterator it;
			it = l->begin();
			advance(it, index);
			l->erase(it);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("LIST_REMOVE_INDEX failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
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
	else {
		MessageBox(HWND_DESKTOP, string("LIST_REMOVE_INDEX_RANGE failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
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
		else {
			MessageBox(HWND_DESKTOP, string("GET_LIST_SIZE failed: list type unknown " + to_string(scriptList->type) + " " + to_string((int)scriptList->intList) + " " + to_string((int)scriptList->floatList) + " " + to_string((int)scriptList->stringList)).c_str(), "CLEO+.cleo", MB_ICONERROR);
			int iSize = -1;
			int fSize = 0;
			int sSize = 0;
			if (scriptList->intList) {
				iSize = scriptList->intList->size();
			}
			if (scriptList->intList) {
				fSize = scriptList->floatList->size();
			}
			if (scriptList->intList) {
				sSize = scriptList->stringList->size();
			}
			MessageBox(HWND_DESKTOP, string(" " + to_string(iSize) + " " + to_string(fSize) + " " + to_string(sSize)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		}
	}
	CLEO_SetIntOpcodeParam(thread, size);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LIST_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		list<int>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
			CLEO_SetIntOpcodeParam(thread, 0);
		}
		else {
			advance(it, index);
			CLEO_SetIntOpcodeParam(thread, *it);
		}
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		list<float>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
			CLEO_SetIntOpcodeParam(thread, 0);
		}
		else {
			advance(it, index);
			CLEO_SetFloatOpcodeParam(thread, *it);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("GET_LIST_VALUE_BY_INDEX failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		CLEO_SetIntOpcodeParam(thread, 0); //fallback
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		list<string>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
			CLEO_WriteStringOpcodeParam(thread, "");
		}
		else {
			advance(it, index);
			string str = *it;
			CLEO_WriteStringOpcodeParam(thread, &str[0]);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("GET_LIST_STRING_VALUE_BY_INDEX failed: list type wrong " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		CLEO_WriteStringOpcodeParam(thread, "");//fallback
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI REPLACE_LIST_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList* scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		int value = CLEO_GetIntOpcodeParam(thread);
		list<int>* l = (list<int>*)scriptList->listPointer;
		list<int>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
		}
		else {
			advance(it, index);
			it = l->erase(it);
			l->insert(it, value);
		}
	}
	else if (scriptList->type == 1) // float
	{
		float value = CLEO_GetFloatOpcodeParam(thread);
		list<float>* l = (list<float>*)scriptList->listPointer;
		list<float>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
		}
		else {
			advance(it, index);
			it = l->erase(it);
			l->insert(it, value);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("REPLACE_LIST_VALUE_BY_INDEX failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		int value = CLEO_GetIntOpcodeParam(thread); //fallback
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI REPLACE_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList* scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	string value = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	if (scriptList->type == 2) // string
	{
		list<string>* l = (list<string>*)scriptList->listPointer;
		list<string>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
		}
		else {
			advance(it, index);
			it = l->erase(it);
			l->insert(it, value);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("REPLACE_LIST_STRING_VALUE_BY_INDEX failed: list type wrong " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI INSERT_LIST_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList* scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		int value = CLEO_GetIntOpcodeParam(thread);
		list<int>* l = (list<int>*)scriptList->listPointer;
		list<int>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
		}
		else {
			advance(it, index);
			l->insert(it, value);
		}
	}
	else if (scriptList->type == 1) // float
	{
		float value = CLEO_GetFloatOpcodeParam(thread);
		list<float>* l = (list<float>*)scriptList->listPointer;
		list<float>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
		}
		else {
			advance(it, index);
			l->insert(it, value);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("INSERT_LIST_VALUE_BY_INDEX failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
		int value = CLEO_GetIntOpcodeParam(thread); //fallback
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI INSERT_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread)
{
	ScriptList* scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	unsigned int index = CLEO_GetIntOpcodeParam(thread);
	string value = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	if (scriptList->type == 2) // string
	{
		list<string>* l = (list<string>*)scriptList->listPointer;
		list<string>::iterator it;
		it = l->begin();
		if (index >= l->size()) {
		}
		else {
			advance(it, index);
			l->insert(it, value);
		}
	}
	else {
		MessageBox(HWND_DESKTOP, string("INSERT_LIST_STRING_VALUE_BY_INDEX failed: list type wrong " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI RESET_LIST(CScriptThread* thread)
{
	ScriptList *scriptList = (ScriptList*)CLEO_GetIntOpcodeParam(thread);
	if (scriptList->type == 0) // int
	{
		list<int> *l = (list<int>*)scriptList->listPointer;
		if (l) l->clear();
	}
	else if (scriptList->type == 1) // float
	{
		list<float> *l = (list<float>*)scriptList->listPointer;
		if (l) l->clear();
	}
	else if (scriptList->type == 2) // string
	{
		list<string> *l = (list<string>*)scriptList->listPointer;
		if (l) l->clear();
	}
	else {
		MessageBox(HWND_DESKTOP, string("RESET_LIST failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
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
	else {
		MessageBox(HWND_DESKTOP, string("REVERSE_LIST failed: list type unknown " + to_string(scriptList->type)).c_str(), "CLEO+.cleo", MB_ICONERROR);
	}
	return OR_CONTINUE;
}
