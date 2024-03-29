#include "OpcodesCommon.h"
#include "CSprite.h"
#include "CEntryExitManager.h"
#include "CMenuManager.h"
#include "CHud.h"
#include "CReplay.h"
#include "CTimer.h"
#include "CCamera.h"

#include "../shared/extensions/Screen.h"

void FixAspectRatio(float *x, float *y)
{
	float resX = (float)RsGlobal.maximumWidth;
	float resY = (float)RsGlobal.maximumHeight;
	resY *= 1.33333333f;
	resX /= resY;

	*x /= resX;
	*y /= 1.07142857f;
}

/////////////////////////////////////////////////

OpcodeResult WINAPI GET_CURRENT_RESOLUTION(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, RsGlobal.maximumWidth);
	CLEO_SetIntOpcodeParam(thread, RsGlobal.maximumHeight);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_FIXED_XY_ASPECT_RATIO(CScriptThread* thread)
{
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	FixAspectRatio(&x, &y);
	CLEO_SetFloatOpcodeParam(thread, x);
	CLEO_SetFloatOpcodeParam(thread, y);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CONVERT_3D_TO_SCREEN_2D(CScriptThread* thread)
{
	bool bResult = false;
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);

	bool checkNearClip = CLEO_GetIntOpcodeParam(thread);
	bool checkFarClip = CLEO_GetFloatOpcodeParam(thread);

	RwV3d screenPos2D;
	float sizeX, sizeY;

	if (CSprite::CalcScreenCoors({ x, y, z }, &screenPos2D, &sizeX, &sizeY, checkFarClip, checkNearClip)) bResult = true;

	x = (screenPos2D.x / RsGlobal.maximumWidth) * 640.0f;
	y = (screenPos2D.y / RsGlobal.maximumHeight) * 448.0f;
	sizeX = (sizeX / RsGlobal.maximumWidth) * 8.0f;
	sizeY = (sizeY / RsGlobal.maximumHeight) * 8.0f;

	CLEO_SetFloatOpcodeParam(thread, x);
	CLEO_SetFloatOpcodeParam(thread, y);
	CLEO_SetFloatOpcodeParam(thread, sizeX);
	CLEO_SetFloatOpcodeParam(thread, sizeY);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_RADAR_VISIBLE(CScriptThread* thread)
{
	bool bResult = false;
	if (CEntryExitManager::ms_exitEnterState != 1
		&& CEntryExitManager::ms_exitEnterState != 2
		&& !CHud::bScriptDontDisplayRadar
		&& FrontEndMenuManager.m_nRadarMode != 2
		&& (CHud::m_ItemToFlash != 8 || CTimer::m_FrameCounter & 8)
		&& CReplay::Mode != 1
		&& !CWeapon::ms_bTakePhoto
		&& (*(unsigned int*)0xB6EC40 != CTimer::m_FrameCounter))
	{
		bResult = true;
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_HUD_VISIBLE(CScriptThread* thread)
{
	bool bResult = false;
	if (FrontEndMenuManager.m_bHudOn
		&& CTheScripts::bDisplayHud
		&& !TheCamera.m_bWideScreenOn
		&& CReplay::Mode != 1
		&& !CWeapon::ms_bTakePhoto
		&& (*(unsigned int*)0xB6EC40 != CTimer::m_FrameCounter))
	{
		bResult = true;
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_FADE_ALPHA(CScriptThread* thread)
{
	float fadeAlpha = TheCamera.m_fFadeAlpha;
	CLEO_SetFloatOpcodeParam(thread, fadeAlpha);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(fadeAlpha > 0.0f);
	return OR_CONTINUE;
}