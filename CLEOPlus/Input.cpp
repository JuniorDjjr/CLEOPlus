#include "OpcodesCommon.h"
#include "extensions/ScriptCommands.h"

#include "CCamera.h"
#include "CPad.h"
#include "CControllerConfigManager.h"
#include "CMenuManager.h"
#include "CCamera.h"

extern bool keysPressedLastFrame[0xFF];
extern bool buttonsPressedLastFrame[2][20];
extern bool disableCamControl;
extern bool disablePadControl[2];
extern bool disablePadControlMovement[2];
extern bool controllerMode;

OpcodeResult WINAPI IS_MOUSE_WHEEL_UP(CScriptThread* thread)
{
	bool bResult = false;
	if (CPad::GetPad(0)->NewMouseControllerState.wheelUp != 0) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_MOUSE_WHEEL_DOWN(CScriptThread* thread)
{
	bool bResult = false;
	if (CPad::GetPad(0)->NewMouseControllerState.wheelDown != 0) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MOUSE_SENSIBILITY(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, TheCamera.m_fMouseAccelHorzntl);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_KEY_JUST_PRESSED(CScriptThread* thread)
{
	int vk = CLEO_GetIntOpcodeParam(thread);
	bool bResult = (((GetKeyState(vk) & 0x8000) != 0) && !keysPressedLastFrame[vk]);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_BUTTON_JUST_PRESSED(CScriptThread* thread)
{
	int pad = CLEO_GetIntOpcodeParam(thread); 
	int id = CLEO_GetIntOpcodeParam(thread);
	bool bResult = ((reinterpret_cast<CRunningScript*>(thread)->GetPadState(pad, id) != 0) && !buttonsPressedLastFrame[pad][id]);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAMERA_CONTROL(CScriptThread* thread)
{
	int controllable = CLEO_GetIntOpcodeParam(thread);
	if (controllable == 0) {
		disableCamControl = true;
	}
	else {
		disableCamControl = false;
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_AIM_BUTTON_PRESSED(CScriptThread* thread)
{
	int pad = CLEO_GetIntOpcodeParam(thread); 
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CPad::GetPad(pad)->GetTarget());
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_PLAYER_CONTROL_PAD(CScriptThread* thread)
{
	int pad = CLEO_GetIntOpcodeParam(thread);
	int set = CLEO_GetIntOpcodeParam(thread);
	disablePadControl[pad] = !set;
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_PLAYER_CONTROL_PAD_MOVEMENT(CScriptThread* thread)
{
	int pad = CLEO_GetIntOpcodeParam(thread);
	int set = CLEO_GetIntOpcodeParam(thread);
	disablePadControlMovement[pad] = !set;
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_ANY_FIRE_BUTTON_PRESSED(CScriptThread* thread)
{
	int pad = CLEO_GetIntOpcodeParam(thread);
	bool bResult = reinterpret_cast<CRunningScript*>(thread)->GetPadState(pad, 17);
	if (!bResult)
	{
		// Secondary (Ginput adapted)
		if (controllerMode == 1)
		{
			bResult = reinterpret_cast<CRunningScript*>(thread)->GetPadState(pad, 4);
		}
		else
		{
			bResult = reinterpret_cast<CRunningScript*>(thread)->GetPadState(pad, 7);
		}
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

// 0xE6E=0,is_select_menu_just_pressed
OpcodeResult WINAPI IS_SELECT_MENU_JUST_PRESSED(CScriptThread* thread)
{
	if (!CCamera::m_bUseMouse3rdPerson)
	{
		reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CPad::GetPad(0)->NewState.ButtonCross && !CPad::GetPad(0)->OldState.ButtonCross);
	}
	else
	{
		int keyCode = ControlsManager.m_actions[13].keys[0].keyCode;
		reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)keyCode));
	}
	return OR_CONTINUE;
}
