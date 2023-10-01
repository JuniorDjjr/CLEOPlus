#include "OpcodesCommon.h"
#include "CCamera.h"
#include "CVisibilityPlugins.h"
#include "CWorld.h"
#include "CCutsceneMgr.h"
#include "CStreaming.h"
#include "CMenuManager.h"
#include "CTimer.h"
#include "CGeneral.h"
#include "CModelInfo.h"
#include "CCheat.h"
#include "CCam.h"
#include "CWeaponInfo.h"
#include "CPedModelInfo.h"
#include "PedExtendedData.h"
#include "VehExtendedData.h"
#include "FxManager_c.h"
#include "Fx_c.h"
#include <set>
#include <time.h>
#include <algorithm>

extern bool inSAMP;
extern set<int> specialCharacterModelsUsed;
extern CStreamingInfo *streamingInfoForModel;
extern uintptr_t addPedModelAddress;
extern tScriptEffectSystem *ScriptEffectSystemArray;

typedef CVector*(__cdecl *VehFuncs_Ext_GetVehicleDummyPosAdapted)(CVehicle * vehicle, int dummyId);
extern VehFuncs_Ext_GetVehicleDummyPosAdapted vehFuncs_Ext_GetVehicleDummyPosAdapted;
extern uint32_t maxDffFiles;
extern char* modelNames;

float VectorSqrMagnitude(CVector vector) { return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z; }

OpcodeResult WINAPI IS_ON_MISSION(CScriptThread* thread)
{
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CTheScripts::OnAMissionFlag && *(scriptSpace + CTheScripts::OnAMissionFlag));
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_ON_SAMP(CScriptThread* thread)
{
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(inSAMP);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_ON_CUTSCENE(CScriptThread* thread)
{
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CCutsceneMgr::ms_running);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_WEAPON_FIRE_TYPE(CScriptThread* thread)
{
	eWeaponType weaponType = (eWeaponType)CLEO_GetIntOpcodeParam(thread);
	int fireType = CLEO_GetIntOpcodeParam(thread);

	bool ok = false;
	auto weaponInfo = CWeaponInfo::GetWeaponInfo(weaponType, 1);

	if (weaponInfo) {
		ok = weaponInfo->m_nWeaponFire == fireType;
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(ok);
	return OR_CONTINUE;
}

OpcodeResult WINAPI FIX_CHAR_GROUND_BRIGHTNESS_AND_FADE_IN(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));

	int ground = CLEO_GetIntOpcodeParam(thread);
	int brightness = CLEO_GetIntOpcodeParam(thread);
	int fade = CLEO_GetIntOpcodeParam(thread);

	if (ped)
	{
		if (fade && ped->m_pRwClump) {
			CVisibilityPlugins::SetClumpAlpha(ped->m_pRwClump, 0);
		}

		CVector *pedPos = nullptr;

		if (ground) {

			CMatrixLink *matrix = ped->m_matrix;
			if (matrix)
				pedPos = &matrix->pos;
			else
				pedPos = &ped->m_placement.m_vPosn;

			bool foundGround;
			CEntity *foundEntity;
			float foundZ = CWorld::FindGroundZFor3DCoord(pedPos->x, pedPos->y, pedPos->z, &foundGround, &foundEntity);
			if (foundGround) {
				pedPos->z = ped->GetDistanceFromCentreOfMassToBaseOfModel() + foundZ;
				ped->SetPosn(*pedPos);
			}
		}

		if (brightness)
		{
			if (!pedPos) {
				CMatrixLink *matrix = ped->m_matrix;
				if (matrix)
					pedPos = &matrix->pos;
				else
					pedPos = &ped->m_placement.m_vPosn;
			}
			
			CEntity *outEntity;
			CColPoint outColPoint;

			if (CWorld::ProcessVerticalLine(*pedPos, -100.0, outColPoint, outEntity, 1, 0, 0, 1, 0, 0, 0))
			{
				char lighting = *(char*)(reinterpret_cast<char*>(&outColPoint) + 0x25); //lightingB (current plugin-sdk is bugged)
				float dayLighting = (float)(lighting & 0xF);
				float nightLighting = (float)(lighting >> 4);
				if (nightLighting < 0.001f) { //workaround for bugging some locations, generally LV at night, I don't know why but maybe there is no night lighting information
					nightLighting = dayLighting * 0.9f;
				}
				else {
					if (dayLighting < 0.001f) { //maybe not required
						dayLighting = nightLighting * 1.1f;
						if (dayLighting > 1.0f) dayLighting = 1.0f;
					}
				}
				ped->m_fContactSurfaceBrightness = dayLighting
					* 0.033333334f
					* (1.0f - *(float*)0x8D12C0)
					+ (float)nightLighting
					* 0.033333334f
					* *(float*)0x8D12C0;
			}
		}
	}
	return OR_CONTINUE;
}

void CPathFind__LoadSceneForPathNodes(float x, float y, float z)
{
	((void *(__stdcall *)(float, float, float))0x44DE00)(x, y, z);
}

OpcodeResult WINAPI SET_CHAR_COORDINATES_SIMPLE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	ped->m_matrix->pos.x = CLEO_GetFloatOpcodeParam(thread);
	ped->m_matrix->pos.y = CLEO_GetFloatOpcodeParam(thread);
	ped->m_matrix->pos.z = CLEO_GetFloatOpcodeParam(thread);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CURRENT_SAVE_SLOT(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, currentSaveSlot + 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_GAME_FIRST_START(CScriptThread* thread)
{
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(timesGameRestarted == 0);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_TEXTURE_FROM_SPRITE(CScriptThread* thread)
{
	int spriteId = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, CLEO_GetScriptTextureById(thread, spriteId));
	return OR_CONTINUE;
}

OpcodeResult WINAPI FRAME_MOD(CScriptThread* thread)
{
	int frame = CLEO_GetIntOpcodeParam(thread);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CTimer::m_FrameCounter % frame == 0);
	return OR_CONTINUE;
}

OpcodeResult WINAPI RANDOM_PERCENT(CScriptThread* thread)
{
	int percent = CLEO_GetIntOpcodeParam(thread);
	int random = CGeneral::GetRandomNumberInRange(0, 100);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(random < percent);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_TRAILER_FROM_CAR(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CVehicle *trailer;
	int trailerRef = -1;
	if (vehicle) {
		trailer = vehicle->m_pTrailer;
		if (trailer != nullptr) {
			trailerRef = CPools::GetVehicleRef(trailer);
		}
	}
	CLEO_SetIntOpcodeParam(thread, trailerRef);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(trailerRef != -1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_FROM_TRAILER(CScriptThread* thread)
{
	CVehicle *trailer = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CVehicle *vehicle;
	int vehicleRef = -1;
	if (trailer) {
		vehicle = trailer->m_pTractor;
		if (vehicle != nullptr) {
			vehicleRef = CPools::GetVehicleRef(vehicle);
		}
	}
	CLEO_SetIntOpcodeParam(thread, vehicleRef);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(vehicleRef != -1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_DUMMY_COORD(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	int dummyId = CLEO_GetIntOpcodeParam(thread);
	int worldCoords = CLEO_GetIntOpcodeParam(thread);
	int invertX = CLEO_GetIntOpcodeParam(thread);
	
	CVector posn = { 0.0f, 0.0f, 0.0f };

	if (vehFuncs_Ext_GetVehicleDummyPosAdapted) {
		CVector *vector = vehFuncs_Ext_GetVehicleDummyPosAdapted(vehicle, dummyId);
		if (!vector) goto GET_CAR_DUMMY_COORD_OFFSET_getByModelInfo;
		posn = *vector;
	}
	else {
		GET_CAR_DUMMY_COORD_OFFSET_getByModelInfo:
		CVehicleModelInfo *vehModelInfo = (CVehicleModelInfo *)CModelInfo::GetModelInfo(vehicle->m_nModelIndex);
		if (vehModelInfo) {
			posn = vehModelInfo->m_pVehicleStruct->m_avDummyPos[dummyId];
		}
	}

	if (posn.x != 0.0f && posn.y != 0.0f && posn.z != 0.0f)
		bResult = true;

	if (invertX)
		posn.x *= -1.0f;

	if (worldCoords)
		posn = vehicle->TransformFromObjectSpace(posn);

	CLEO_SetFloatOpcodeParam(thread, posn.x);
	CLEO_SetFloatOpcodeParam(thread, posn.y);
	CLEO_SetFloatOpcodeParam(thread, posn.z);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CHEAT_ACTIVE(CScriptThread* thread)
{
	int i = CLEO_GetIntOpcodeParam(thread);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(CCheat::m_aCheatsActive[i]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CHANGE_PLAYER_MONEY(CScriptThread* thread)
{
	int player = CLEO_GetIntOpcodeParam(thread);
	int mode = CLEO_GetIntOpcodeParam(thread);
	int value = CLEO_GetIntOpcodeParam(thread);
	CPlayerPed* playerPed = FindPlayerPed(player);
	if (playerPed) {
		CPlayerInfo *playerInfo = playerPed->GetPlayerInfoForThisPlayerPed();
		if (playerInfo) {
			switch (mode) {
			case 0:
				playerInfo->m_nMoney = value;
				playerInfo->m_nDisplayMoney = value;
				break;
			case 1:
				playerInfo->m_nMoney += value;
				break;
			case 2:
				playerInfo->m_nMoney -= value;
				break;
			}
		}
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI CAR_HORN(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	if (vehicle) {
		vehicle->m_nHornCounter = 1;
		vehicle->PlayCarHorn();
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_STRING_LENGTH(CScriptThread* thread)
{
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	int len = strnlen_s(string, 128);
	CLEO_SetIntOpcodeParam(thread, len);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(len > 0);
	return OR_CONTINUE;
}

OpcodeResult WINAPI COPY_STRING(CScriptThread* thread)
{
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	int size = strnlen_s(string, 128);
	char* pointer;
	auto paramType = *reinterpret_cast<CRunningScript*>(thread)->m_pCurrentIP;
	if (paramType >= 1 && paramType <= 8) {
		pointer = (char*)CLEO_GetIntOpcodeParam(thread);
	}
	else {
		pointer = (char*)CLEO_GetPointerToScriptVariable(thread);
	}
	memcpy_s(pointer, size, string, size);
	*(char*)(pointer + size) = 0;
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAR_ALARM(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	int mode = CLEO_GetIntOpcodeParam(thread);
	if (mode == 0) {
		vehicle->m_nAlarmState = 0;
	}
	else {
		if (mode == 1) {
			vehicle->m_nAlarmState = 65535;
		}
		else {
			if (mode == 2) {
				vehicle->m_nAlarmState = 65534;
			}
		}
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_ALARM(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	int mode = -1;
	if (vehicle->m_nAlarmState == 0) {
		mode = 0;
	}
	else {
		if (vehicle->m_nAlarmState == 65535) {
			mode = 1;
		}
		else {
			if (vehicle->m_nAlarmState < 65535) {
				mode = 2;
			}
		}
	}
	CLEO_SetIntOpcodeParam(thread, mode);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_MAX_HEALTH(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	int pedType = ped->m_nPedType;
	float maxHealth;
	if (pedType <= 1) { // is player
		maxHealth = (float)CWorld::Players[pedType].m_nMaxHealth;
	}
	else {
		maxHealth = ped->m_fMaxHealth;
	}
	CLEO_SetFloatOpcodeParam(thread, maxHealth);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_HEALTH_PERCENT(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	int pedType = ped->m_nPedType;
	float maxHealth;
	if (pedType <= 1) { // is player
		maxHealth = (float)CWorld::Players[pedType].m_nMaxHealth;
	}
	else {
		maxHealth = ped->m_fMaxHealth;
	}
	float curHealth = ped->m_fHealth;
	float fCurHealthPercent = (curHealth / maxHealth) * 100.0f;
	CLEO_SetFloatOpcodeParam(thread, fCurHealthPercent);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CURRENT_CAMERA_MODE(CScriptThread* thread)
{
	int mode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;
	CLEO_SetIntOpcodeParam(thread, mode);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_COLLISION_INTENSITY(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	float intensity = vehicle->m_fDamageIntensity;
	CLEO_SetFloatOpcodeParam(thread, intensity);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(intensity > 0.0f);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_COLLISION_COORDINATES(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CVector pos = vehicle->m_vecLastCollisionPosn;
	CLEO_SetFloatOpcodeParam(thread, pos.x);
	CLEO_SetFloatOpcodeParam(thread, pos.y);
	CLEO_SetFloatOpcodeParam(thread, pos.z);
	return OR_CONTINUE;
}

OpcodeResult WINAPI STREAM_CUSTOM_SCRIPT_FROM_LABEL(CScriptThread* thread)
{
	int label = CLEO_GetIntOpcodeParam(thread);
	CLEO_CreateCustomScript(thread, thread->threadName, label);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LAST_CREATED_CUSTOM_SCRIPT(CScriptThread* thread)
{
	CScriptThread* cs = CLEO_GetLastCreatedCustomScript();
	CLEO_SetIntOpcodeParam(thread, (DWORD)cs);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(cs != nullptr);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAR_MODEL_ALPHA(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	unsigned int alpha = CLEO_GetIntOpcodeParam(thread);
	if (vehicle->m_pRwObject)
	{
		vehicle->SetRwObjectAlpha(alpha);
		bResult = true;
	}
	else bResult = false;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CHAR_MODEL_ALPHA(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	unsigned int alpha = CLEO_GetIntOpcodeParam(thread);
	if (ped->m_pRwObject)
	{
		ped->SetRwObjectAlpha(alpha);
		bResult = true;
	}
	else bResult = false;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_OBJECT_MODEL_ALPHA(CScriptThread* thread)
{
	bool bResult = false;
	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	unsigned int alpha = CLEO_GetIntOpcodeParam(thread);
	if (obj->m_pRwObject)
	{
		obj->SetRwObjectAlpha(alpha);
		bResult = true;
	}
	else bResult = false;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LOCAL_TIME(CScriptThread* thread)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CLEO_SetIntOpcodeParam(thread, time.wYear);
	CLEO_SetIntOpcodeParam(thread, time.wMonth);
	CLEO_SetIntOpcodeParam(thread, time.wDayOfWeek);
	CLEO_SetIntOpcodeParam(thread, time.wDay);
	CLEO_SetIntOpcodeParam(thread, time.wHour);
	CLEO_SetIntOpcodeParam(thread, time.wMinute);
	CLEO_SetIntOpcodeParam(thread, time.wSecond);
	CLEO_SetIntOpcodeParam(thread, time.wMilliseconds);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_SCRIPT_VAR(CScriptThread* thread)
{
	void *t, *value; unsigned int var; SCRIPT_VAR scrVal;
	t = (void *)CLEO_GetIntOpcodeParam(thread);
	var = CLEO_GetIntOpcodeParam(thread);
	value = (void *)CLEO_GetIntOpcodeParam(thread);
	scrVal.pParam = value;
	((CScriptThread *)t)->tls[var] = scrVal;
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_SCRIPT_VAR(CScriptThread* thread)
{
	void *t; unsigned int var;
	t = (void *)CLEO_GetIntOpcodeParam(thread);
	var = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)((CScriptThread *)t)->tls[var].pParam);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAR_DOOR_WINDOW_STATE(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	unsigned int door = CLEO_GetIntOpcodeParam(thread);
	unsigned int state = CLEO_GetIntOpcodeParam(thread);
	if (state) vehicle->SetWindowOpenFlag(door); else vehicle->ClearWindowOpenFlag(door);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_OBJECT_CENTRE_OF_MASS_TO_BASE_OF_MODEL(CScriptThread* thread)
{
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetFloatOpcodeParam(thread, object->GetDistanceFromCentreOfMassToBaseOfModel());
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MODEL_TYPE(CScriptThread* thread)
{
	int modelId = CLEO_GetIntOpcodeParam(thread);
	CBaseModelInfo* modelInfo;
	int type = -1;
	if (modelId >= 0) {
		modelInfo = CModelInfo::GetModelInfo(modelId);
		if (modelInfo) {
			type = modelInfo->GetModelType();
		}
	}
	CLEO_SetIntOpcodeParam(thread, type);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_STRING_EQUAL(CScriptThread* thread)
{
	bool bResult = true;
	LPSTR stringA = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	LPSTR stringB = CLEO_ReadStringPointerOpcodeParam(thread, bufferB, 128);
	int maxSize = CLEO_GetIntOpcodeParam(thread);
	int caseSensitive = CLEO_GetIntOpcodeParam(thread);
	LPSTR ignoreCharactere = CLEO_ReadStringPointerOpcodeParam(thread, bufferC, 1);

	if (!caseSensitive)
	{
		_strupr(stringA);
		_strupr(stringB);
	}

	int i = 0;
	while (i < maxSize && stringA[i] != 0 && stringB[i] != 0)
	{
		if (stringA[i] != ignoreCharactere[0] && stringB[i] != ignoreCharactere[0])
		{
			if (stringA[i] != stringB[i])
			{
				bResult = false;
				break;
			}
		}
		++i;
	}

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_STRING_COMMENT(CScriptThread* thread)
{
	bool bResult = false;
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);

	unsigned int i = 0;
	while (string[i] == ' ' && i <= 127) ++i;

	if (string[i] == '#' || string[i] == ';' || (string[i] == '/' && string[i + 1] == '/')) bResult = true;

	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DOES_CAR_HAVE_PART_NODE(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	int nodeId = CLEO_GetIntOpcodeParam(thread);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(reinterpret_cast<CAutomobile*>(vehicle)->m_aCarNodes[nodeId] != nullptr);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CURRENT_CHAR_WEAPONINFO(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	eWeaponType weaponType = ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType;
	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(weaponType, ped->GetWeaponSkill(weaponType));
	CLEO_SetIntOpcodeParam(thread, (DWORD)weaponInfo);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(weaponInfo != nullptr);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO(CScriptThread* thread)
{
	int weaponId = CLEO_GetIntOpcodeParam(thread);
	int skill = CLEO_GetIntOpcodeParam(thread);
	CWeaponInfo* weaponInfo = nullptr;

	if (weaponId >= 70 || skill < 0 || skill > 3) 
	{
		// THIS IS UNSAFE!
		weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponId, skill);
	}
	else
	{
		if (weaponId >= 22 && weaponId <= 32) {
			weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponId, skill);
		}
		else {
			weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponId, 1);
		}
	}

	CLEO_SetIntOpcodeParam(thread, (DWORD)weaponInfo);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(weaponInfo != nullptr);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO_MODELS(CScriptThread* thread)
{
	CWeaponInfo *weaponInfo = (CWeaponInfo *)CLEO_GetIntOpcodeParam(thread);
	int modelA = weaponInfo->m_nModelId1;
	int modelB = weaponInfo->m_nModelId2;
	CLEO_SetIntOpcodeParam(thread, modelA);
	CLEO_SetIntOpcodeParam(thread, modelB);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO_FLAGS(CScriptThread* thread)
{
	unsigned int weaponInfo = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, *(DWORD*)(weaponInfo + 0x18));
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO_ANIMGROUP(CScriptThread* thread)
{
	CWeaponInfo *weaponInfo = (CWeaponInfo *)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)weaponInfo->m_nAnimToPlay);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO_TOTAL_CLIP(CScriptThread* thread)
{
	CWeaponInfo *weaponInfo = (CWeaponInfo *)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)weaponInfo->m_nAmmoClip);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO_FIRE_TYPE(CScriptThread* thread)
{
	CWeaponInfo *weaponInfo = (CWeaponInfo *)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)weaponInfo->m_nWeaponFire);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_WEAPONINFO_SLOT(CScriptThread* thread)
{
	CWeaponInfo *weaponInfo = (CWeaponInfo *)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)weaponInfo->m_nSlot);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_WEAPON_STATE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_nState);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_WEAPON_CLIP(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_nAmmoInClip);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_COLLISION_SURFACE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, ped->m_nContactSurface);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_COLLISION_LIGHTING(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetFloatOpcodeParam(thread, ped->m_fContactSurfaceBrightness);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_COLLISION_SURFACE(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, vehicle->m_nContactSurface);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_COLLISION_LIGHTING(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetFloatOpcodeParam(thread, vehicle->m_fContactSurfaceBrightness);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CHAR_REALLY_IN_AIR(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(!ped->m_nPhysicalFlags.bOnSolidSurface && !ped->m_nPhysicalFlags.bSubmergedInWater);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CAR_REALLY_IN_AIR(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(!vehicle->m_nPhysicalFlags.bOnSolidSurface && !vehicle->m_nPhysicalFlags.bSubmergedInWater);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_OBJECT_REALLY_IN_AIR(CScriptThread* thread)
{
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(!object->m_nPhysicalFlags.bOnSolidSurface && !object->m_nPhysicalFlags.bSubmergedInWater);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SIMULATE_OBJECT_DAMAGE(CScriptThread* thread)
{
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	float damage = CLEO_GetFloatOpcodeParam(thread);
	int type = CLEO_GetIntOpcodeParam(thread);
	object->ObjectDamage(damage, nullptr, nullptr, nullptr, (eWeaponType)type);
	return OR_CONTINUE;
}

OpcodeResult WINAPI REQUEST_PRIORITY_MODEL(CScriptThread* thread)
{
	int model = CLEO_GetIntOpcodeParam(thread);
	CStreaming::RequestModel(model, eStreamingFlags::KEEP_IN_MEMORY | eStreamingFlags::MISSION_REQUIRED | eStreamingFlags::PRIORITY_REQUEST);
	CTheScripts::ScriptResourceManager.AddToResourceManager(model, 2, reinterpret_cast<CRunningScript*>(thread));
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOAD_ALL_PRIORITY_MODELS_NOW(CScriptThread* thread)
{
	//CStreaming::LoadAllRequestedModels(true);
	CTimer::Suspend();
	CStreaming::LoadAllRequestedModels(false); // "true" doesn't work, I don't know why, so, now it works just like LOAD_ALL_MODELS_NOW
	CTimer::Resume();
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOAD_SPECIAL_CHARACTER_FOR_ID(CScriptThread* thread)
{
	int id = CLEO_GetIntOpcodeParam(thread);
	CBaseModelInfo *baseModelInfo = CModelInfo::GetModelInfo(id);
	if (!baseModelInfo) {

		CPedModelInfo *pedModelInfo = ((CPedModelInfo* (__cdecl *)(int))addPedModelAddress)(id);
		//CPedModelInfo *pedModelInfo = CModelInfo::AddPedModel(id);
		pedModelInfo->SetColModel((CColModel*)0x968DF0, 0);
		CPedModelInfo *basePedModelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(290);
		pedModelInfo->m_nPedType = ePedType::PED_TYPE_CIVMALE;
		pedModelInfo->m_nStatType = 41;
		if (basePedModelInfo) {
			pedModelInfo->m_nAnimType = basePedModelInfo->m_nAnimType;
			pedModelInfo->m_nCarsCanDriveMask = basePedModelInfo->m_nCarsCanDriveMask;
			pedModelInfo->m_nPedFlags = basePedModelInfo->m_nPedFlags;
			pedModelInfo->m_nRadio1 = basePedModelInfo->m_nRadio1;
			pedModelInfo->m_nRadio2 = basePedModelInfo->m_nRadio2;
			pedModelInfo->m_nRace = basePedModelInfo->m_nRace;
			pedModelInfo->m_nPedAudioType = basePedModelInfo->m_nPedAudioType;
			pedModelInfo->m_nVoice1 = basePedModelInfo->m_nVoice1;
			pedModelInfo->m_nVoice2 = basePedModelInfo->m_nVoice2;
			pedModelInfo->m_nVoiceId = basePedModelInfo->m_nVoiceId;
		}
	}
	specialCharacterModelsUsed.insert(id);
	LPSTR name = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	CStreaming::RequestSpecialModel(id, name, eStreamingFlags::KEEP_IN_MEMORY | eStreamingFlags::MISSION_REQUIRED);
	CTheScripts::ScriptResourceManager.AddToResourceManager(id, 2, reinterpret_cast<CRunningScript*>(thread));
	return OR_CONTINUE;
}

OpcodeResult WINAPI UNLOAD_SPECIAL_CHARACTER_FROM_ID(CScriptThread* thread)
{
	int id = CLEO_GetIntOpcodeParam(thread);
	if (CTheScripts::ScriptResourceManager.RemoveFromResourceManager(id, 2, reinterpret_cast<CRunningScript*>(thread))) {
		CStreaming::SetMissionDoesntRequireModel(id);
	}
	CBaseModelInfo *baseModelInfo = CModelInfo::GetModelInfo(id);
	if (baseModelInfo) {
		if (baseModelInfo->m_nRefCount == 0) {
			CStreaming::RemoveModel(id);
		}
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MODEL_BY_NAME(CScriptThread* thread)
{
	LPSTR name = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	int id = -1;
	CBaseModelInfo *baseModelInfo = CModelInfo::GetModelInfo(name, &id);
	CLEO_SetIntOpcodeParam(thread, id);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(baseModelInfo && id > -1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_MODEL_AVAILABLE_BY_NAME(CScriptThread* thread)
{
	LPSTR name = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	unsigned int offset, size;
	CDirectory::DirectoryInfo *dir = CStreaming::ms_pExtraObjectsDir->FindItem((const char*)name, offset, size);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(dir);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MODEL_DOESNT_EXIST_IN_RANGE(CScriptThread* thread)
{
	bool bResult = false;
	int start = CLEO_GetIntOpcodeParam(thread);
	int end = CLEO_GetIntOpcodeParam(thread);

	int id;
	for (id = start; id <= end; ++id) {
		if (plugin::CallAndReturn<bool, 0x407800, int>(id) == false || (streamingInfoForModel[id].m_nLoadState != 1 && specialCharacterModelsUsed.find(id) != specialCharacterModelsUsed.end())) {
			bResult = true;
			break;
		}
	}

	if (!bResult) id = -1;
	CLEO_SetIntOpcodeParam(thread, id);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI REMOVE_ALL_UNUSED_MODELS(CScriptThread* thread)
{
	CStreaming::RemoveAllUnusedModels();
	return OR_CONTINUE;
}

OpcodeResult WINAPI REMOVE_MODEL_IF_UNUSED(CScriptThread* thread)
{
	int id = CLEO_GetIntOpcodeParam(thread);
	CBaseModelInfo *baseModelInfo = CModelInfo::GetModelInfo(id);
	if (baseModelInfo) {
		if (baseModelInfo->m_nRefCount == 0) {
			CStreaming::RemoveModel(id);
		}
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CHAR_ON_FIRE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(ped->m_pFire != nullptr);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CLOSEST_COP_NEAR_CHAR(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	int alive = CLEO_GetIntOpcodeParam(thread);
	int inCar = CLEO_GetIntOpcodeParam(thread);
	int onFoot = CLEO_GetIntOpcodeParam(thread);
	int seenInFront = CLEO_GetIntOpcodeParam(thread);
	
	CVector pos = ped->GetPosition();
	auto& pool = CPools::ms_pPedPool;

	CPed *closestCop = nullptr;
	for (int index = 0; index < pool->m_nSize; ++index)
	{
		if (auto obj = pool->GetAt(index))
		{
			if (ped != obj)
			{
				if (obj->m_pVehicle) {
					if (onFoot && !inCar) continue;
				}
				else {
					if (inCar && !onFoot) continue;
				}
				if (!alive || obj->m_fHealth > 0.0f)
				{
					if (radius >= 1000.0f || (VectorSqrMagnitude(obj->GetPosition() - pos) <= radius * radius))
					{
						// Also checks for models, because the game itself don`t use correct pedtype on dep garage
						if (obj->m_nPedType == ePedType::PED_TYPE_COP || (obj->m_nModelIndex >= 280 && obj->m_nModelIndex <= 288))
						{
							if (!seenInFront || obj->OurPedCanSeeThisEntity(ped, false))
							{
								// Update closest one
								if (closestCop)
								{
									if ((VectorSqrMagnitude(closestCop->GetPosition() - pos)) < (VectorSqrMagnitude(obj->GetPosition() - pos)))
									{
										continue;
									}
								}
								closestCop = obj;
							}
						}
					}
				}
			}
		}
	}

	int returnCop = -1;
	if (closestCop) {
		returnCop = CPools::GetPedRef(closestCop);
		bResult = true;
	}

	CLEO_SetIntOpcodeParam(thread, returnCop);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CLOSEST_COP_NEAR_POS(CScriptThread* thread)
{
	bool bResult = false;
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	float radius = CLEO_GetFloatOpcodeParam(thread);
	int alive = CLEO_GetIntOpcodeParam(thread);
	int inCar = CLEO_GetIntOpcodeParam(thread);
	int onFoot = CLEO_GetIntOpcodeParam(thread);

	CVector pos = { x, y, z, };
	auto& pool = CPools::ms_pPedPool;

	CPed *closestCop = nullptr;
	for (int index = 0; index < pool->m_nSize; ++index)
	{
		if (auto obj = pool->GetAt(index))
		{
			if (obj->m_pVehicle) {
				if (onFoot && !inCar) continue;
			}
			else {
				if (inCar && !onFoot) continue;
			}
			if (!alive || obj->m_fHealth > 0.0f)
			{
				if (radius >= 1000.0f || (VectorSqrMagnitude(obj->GetPosition() - pos) <= radius * radius))
				{
					// Also checks for models, because the game itself don`t use correct pedtype on dep garage
					if (obj->m_nPedType == ePedType::PED_TYPE_COP || (obj->m_nModelIndex >= 280 && obj->m_nModelIndex <= 288))
					{
						// Update closest one
						if (closestCop)
						{
							if ((VectorSqrMagnitude(closestCop->GetPosition() - pos)) < (VectorSqrMagnitude(obj->GetPosition() - pos)))
							{
								continue;
							}
						}
						closestCop = obj;
					}
				}
			}
		}
	}

	int returnCop = -1;
	if (closestCop) {
		returnCop = CPools::GetPedRef(closestCop);
		bResult = true;
	}

	CLEO_SetIntOpcodeParam(thread, returnCop);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ANY_CHAR_NO_SAVE_RECURSIVE(CScriptThread* thread)
{
	bool bResult = false;
	int progress = CLEO_GetIntOpcodeParam(thread);

	auto& pool = CPools::ms_pPedPool;

	CPed *objFound = nullptr;
	for (int index = progress; index < pool->m_nSize; ++index)
	{
		if (auto obj = pool->GetAt(index))
		{
			progress = index + 1;
			objFound = obj;
			break;
		}
	}

	int objReturn = -1;
	if (objFound) {
		objReturn = CPools::GetPedRef(objFound);
		bResult = true;
	}

	CLEO_SetIntOpcodeParam(thread, progress);
	CLEO_SetIntOpcodeParam(thread, objReturn);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ANY_CAR_NO_SAVE_RECURSIVE(CScriptThread* thread)
{
	bool bResult = false;
	int progress = CLEO_GetIntOpcodeParam(thread);

	auto& pool = CPools::ms_pVehiclePool;

	CVehicle *objFound = nullptr;
	for (int index = progress; index < pool->m_nSize; ++index)
	{
		if (auto obj = pool->GetAt(index))
		{
			progress = index + 1;
			objFound = obj;
			break;
		}
	}

	int objReturn = -1;
	if (objFound) {
		objReturn = CPools::GetVehicleRef(objFound);
		bResult = true;
	}

	CLEO_SetIntOpcodeParam(thread, progress);
	CLEO_SetIntOpcodeParam(thread, objReturn);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ANY_OBJECT_NO_SAVE_RECURSIVE(CScriptThread* thread)
{
	bool bResult = false;
	int progress = CLEO_GetIntOpcodeParam(thread);

	auto& pool = CPools::ms_pObjectPool;

	CObject *objFound = nullptr;
	for (int index = progress; index < pool->m_nSize; ++index)
	{
		if (auto obj = pool->GetAt(index))
		{
			progress = index + 1;
			objFound = obj;
			break;
		}
	}

	int objReturn = -1;
	if (objFound) {
		objReturn = CPools::GetObjectRef(objFound);
		bResult = true;
	}

	CLEO_SetIntOpcodeParam(thread, progress);
	CLEO_SetIntOpcodeParam(thread, objReturn);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CHAR_ARRESTED(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	ped->m_nPedState = ePedState::PEDSTATE_ARRESTED;
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_PEDSTATE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, (DWORD)ped->m_nPedState);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_PROOFS(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, (DWORD)ped->m_nPhysicalFlags.bBulletProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)ped->m_nPhysicalFlags.bFireProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)ped->m_nPhysicalFlags.bExplosionProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)ped->m_nPhysicalFlags.bCollisionProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)ped->m_nPhysicalFlags.bMeleeProof);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_PROOFS(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, (DWORD)vehicle->m_nPhysicalFlags.bBulletProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)vehicle->m_nPhysicalFlags.bFireProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)vehicle->m_nPhysicalFlags.bExplosionProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)vehicle->m_nPhysicalFlags.bCollisionProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)vehicle->m_nPhysicalFlags.bMeleeProof);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_OBJECT_PROOFS(CScriptThread* thread)
{
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, (DWORD)object->m_nPhysicalFlags.bBulletProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)object->m_nPhysicalFlags.bFireProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)object->m_nPhysicalFlags.bExplosionProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)object->m_nPhysicalFlags.bCollisionProof);
	CLEO_SetIntOpcodeParam(thread, (DWORD)object->m_nPhysicalFlags.bMeleeProof);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CHAR_WEAPON_VISIBLE_SET(CScriptThread* thread)
{
	bool bResult = true;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CPlayerData *playerData = ped->m_pPlayerData;
	if (playerData) {
		if (!playerData->m_bRenderWeapon) bResult = false;
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_STAT_ID(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, *(DWORD*)ped->m_pStats);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS(CScriptThread* thread)
{
	int camID = TheCamera.m_nActiveCam;
	
	CVector offset;
	offset.x = CLEO_GetFloatOpcodeParam(thread);
	offset.y = CLEO_GetFloatOpcodeParam(thread);
	offset.z = CLEO_GetFloatOpcodeParam(thread);

	CVector posReturn;
	CMatrixLink* matrix = TheCamera.GetMatrix();
	if (matrix) {
		RwV3dTransformPoint((RwV3d*)&posReturn, (RwV3d*)&offset, (RwMatrix*)matrix);

		CLEO_SetFloatOpcodeParam(thread, posReturn.x);
		CLEO_SetFloatOpcodeParam(thread, posReturn.y);
		CLEO_SetFloatOpcodeParam(thread, posReturn.z);
	}
	else {
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_OFFSET_FROM_MATRIX_IN_WORLD_COORDS(CScriptThread* thread)
{
	CMatrix *matrix = (CMatrix *)CLEO_GetIntOpcodeParam(thread);
	
	CVector offset;
	offset.x = CLEO_GetFloatOpcodeParam(thread);
	offset.y = CLEO_GetFloatOpcodeParam(thread);
	offset.z = CLEO_GetFloatOpcodeParam(thread);

	CVector posReturn;

	RwV3dTransformPoint((RwV3d*)&posReturn, (RwV3d*)&offset, (RwMatrix*)matrix);

	CLEO_SetFloatOpcodeParam(thread, posReturn.x);
	CLEO_SetFloatOpcodeParam(thread, posReturn.y);
	CLEO_SetFloatOpcodeParam(thread, posReturn.z);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAR_COORDINATES_SIMPLE(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	vehicle->m_matrix->pos.x = CLEO_GetFloatOpcodeParam(thread);
	vehicle->m_matrix->pos.y = CLEO_GetFloatOpcodeParam(thread);
	vehicle->m_matrix->pos.z = CLEO_GetFloatOpcodeParam(thread);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_DAMAGE_LAST_FRAME(CScriptThread* thread)
{
	bool bResult = false;
	int ref = CLEO_GetIntOpcodeParam(thread);
	CPed *ped = CPools::GetPed(ref);
	PedExtended &data = extData.Get(ped);
	if (&data != nullptr)
	{
		CLEO_SetIntOpcodeParam(thread, (DWORD)data.lastDamageEntity);
		CLEO_SetIntOpcodeParam(thread, data.lastDamageWeapon);
		CLEO_SetIntOpcodeParam(thread, data.lastDamagePart);
		CLEO_SetFloatOpcodeParam(thread, data.lastDamageIntensity);
		if (data.lastDamageIntensity > 0.0f) bResult = true;
	}
	else
	{
		CLEO_SetIntOpcodeParam(thread, -1);
		CLEO_SetIntOpcodeParam(thread, -1);
		CLEO_SetIntOpcodeParam(thread, -1);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_WEAPON_DAMAGE_LAST_FRAME(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	VehExtended &data = vehExtData.Get(vehicle);

	int pedReturn = -1;

	if (&data != nullptr) {
		if (data.lastDamagePed) {
			pedReturn = CPools::GetPedRef((CPed*)data.lastDamagePed);
		}
		if (data.lastDamageIntensity != 0.0f) bResult = true;

		CLEO_SetIntOpcodeParam(thread, pedReturn);
		CLEO_SetIntOpcodeParam(thread, data.lastDamageType);
		CLEO_SetFloatOpcodeParam(thread, data.lastDamageIntensity);
	}
	else {
		CLEO_SetIntOpcodeParam(thread, pedReturn);
		CLEO_SetIntOpcodeParam(thread, -1);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_ON_SCRIPTED_CUTSCENE(CScriptThread* thread)
{
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(TheCamera.m_bWideScreenOn);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MODEL_PED_TYPE_AND_STAT(CScriptThread* thread)
{
	bool bResult = false;
	int model = CLEO_GetIntOpcodeParam(thread);
	CPedModelInfo *pedModelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(model);
	int type = -1;
	int stat = -1;
	if (pedModelInfo) {
		type = pedModelInfo->m_nPedType;
		stat = pedModelInfo->m_nStatType;
		bResult = true;
	}
	CLEO_SetIntOpcodeParam(thread, type);
	CLEO_SetIntOpcodeParam(thread, stat);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI PASS_TIME(CScriptThread* thread)
{
	int mins = CLEO_GetIntOpcodeParam(thread);
	plugin::Call<0x4414C0, int>(mins); // void __cdecl CGameLogic::PassTime(unsigned int a1)
	return OR_CONTINUE;
}

OpcodeResult WINAPI GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED(CScriptThread* thread)
{
	int seed = CLEO_GetIntOpcodeParam(thread);
	int min = CLEO_GetIntOpcodeParam(thread);
	int max = CLEO_GetIntOpcodeParam(thread) - 1;
	unsigned int resetSeed = rand() / 2;
	srand(seed);
	CLEO_SetIntOpcodeParam(thread, Random(min, max));
	srand((time(0) / 2) + resetSeed); // reset
	return OR_CONTINUE;
}
 
OpcodeResult WINAPI GENERATE_RANDOM_FLOAT_IN_RANGE_WITH_SEED(CScriptThread* thread)
{
	int seed = CLEO_GetIntOpcodeParam(thread);
	float min = CLEO_GetFloatOpcodeParam(thread);
	float max = CLEO_GetFloatOpcodeParam(thread);
	unsigned int resetSeed = rand() / 2;
	srand(seed);
	CLEO_SetFloatOpcodeParam(thread, fmaf((float)rand() / RAND_MAX, max - min, min));
	srand((time(0) / 2) + resetSeed); // reset
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CAMERA_DISTANCE_TO_COORDINATES(CScriptThread* thread)
{
	bool bResult = false;
	CVector pos;
	pos.x = CLEO_GetFloatOpcodeParam(thread);
	pos.y = CLEO_GetFloatOpcodeParam(thread);
	pos.z = CLEO_GetFloatOpcodeParam(thread);
	float radius = CLEO_GetFloatOpcodeParam(thread);
	if (VectorSqrMagnitude(TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecSource - pos) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_FX_SYSTEM_POINTER(CScriptThread* thread)
{
	int handle = CLEO_GetIntOpcodeParam(thread);
	if (handle >= 0) {
		handle = CTheScripts::GetActualScriptThingIndex(handle, 1);
		if (handle >= 0) {
			CLEO_SetIntOpcodeParam(thread, (DWORD)ScriptEffectSystemArray[handle].m_pFxSystem);
			reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(true);
			return OR_CONTINUE;
		}
	}
	CLEO_SetIntOpcodeParam(thread, 0);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(false);
	return OR_CONTINUE;
}

OpcodeResult WINAPI ADD_FX_SYSTEM_PARTICLE(CScriptThread* thread)
{
	int handle = CLEO_GetIntOpcodeParam(thread);

	RwV3d pos;
	pos.x = CLEO_GetFloatOpcodeParam(thread);
	pos.y = CLEO_GetFloatOpcodeParam(thread);
	pos.z = CLEO_GetFloatOpcodeParam(thread);

	RwV3d vel;
	vel.x = CLEO_GetFloatOpcodeParam(thread);
	vel.y = CLEO_GetFloatOpcodeParam(thread);
	vel.z = CLEO_GetFloatOpcodeParam(thread);

	float size = CLEO_GetFloatOpcodeParam(thread);

	float brightness = CLEO_GetFloatOpcodeParam(thread);

	float r = CLEO_GetFloatOpcodeParam(thread);
	float g = CLEO_GetFloatOpcodeParam(thread);
	float b = CLEO_GetFloatOpcodeParam(thread);
	float a = CLEO_GetFloatOpcodeParam(thread);

	float lastFactor = CLEO_GetFloatOpcodeParam(thread);

	if (handle >= 0) {
		handle = CTheScripts::GetActualScriptThingIndex(handle, 1);
		if (handle >= 0) {
			FxSystem_c *fx = (FxSystem_c *)ScriptEffectSystemArray[handle].m_pFxSystem;

			FxPrtMult_c *prtMult = new FxPrtMult_c(r, g, b, a, size, 0.0, lastFactor);

			fx->AddParticle(&pos, &vel, 0.0f, prtMult, -1.0f, brightness, 0.0f, (unsigned char)0);
		}
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_FX_SYSTEM_AVAILABLE_WITH_NAME(CScriptThread* thread)
{
	bool bResult = false;
	LPSTR string = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	FxSystemBP_c *blueprint = g_fxMan.FindFxSystemBP(string);
	if (blueprint) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_STRING_UPPER(CScriptThread* thread)
{
	char *string = (char *)CLEO_GetIntOpcodeParam(thread);
	_strupr(string);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_STRING_LOWER(CScriptThread* thread)
{
	char *string = (char *)CLEO_GetIntOpcodeParam(thread);
	_strlwr(string);
	return OR_CONTINUE;
}

OpcodeResult WINAPI STRING_FIND(CScriptThread* thread)
{
	bool bResult = false;
	int mode = CLEO_GetIntOpcodeParam(thread);
	std::string str = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	std::string strFind = CLEO_ReadStringPointerOpcodeParam(thread, bufferB, 128);

	size_t found = string::npos;

	if (mode == 0)
	{
		found = str.find(strFind);
	}
	else if (mode == 1)
	{
		found = str.rfind(strFind);
	}

	if (found != string::npos)
	{
		bResult = true;
		CLEO_SetIntOpcodeParam(thread, found);
	}
	else
	{
		CLEO_SetIntOpcodeParam(thread, -1);
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CUT_STRING_AT(CScriptThread* thread)
{
	char *string = (char *)CLEO_GetIntOpcodeParam(thread);
	string[CLEO_GetIntOpcodeParam(thread)] = 0;
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_STRING_CHARACTER_AT(CScriptThread* thread)
{
	bool bResult = false;
	LPSTR str = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	LPSTR cha = CLEO_ReadStringPointerOpcodeParam(thread, bufferC, 1);
	int index = CLEO_GetIntOpcodeParam(thread);
	if (str[index] == cha[0]) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_RANDOM_SEED(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, ped->m_nRandomSeed);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_RANDOM_SEED(CScriptThread* thread)
{
	CVehicle *veh = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, veh->m_nRandomSeed);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_OBJECT_RANDOM_SEED(CScriptThread* thread)
{
	CObject *obj = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, obj->m_nRandomSeed);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_MOVE_STATE(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, ped->m_nMoveState);
	return OR_CONTINUE;
}

OpcodeResult WINAPI DONT_DELETE_CHAR_UNTIL_TIME(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	unsigned int time = CLEO_GetIntOpcodeParam(thread) + CTimer::m_snTimeInMilliseconds;
	if ((unsigned int)ped->field_54C < time) { // m_nTimeTillWeNeedThisPed
		ped->field_54C = time;
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI DONT_DELETE_CAR_UNTIL_TIME(CScriptThread* thread)
{
	CVehicle *veh = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	unsigned int time = CLEO_GetIntOpcodeParam(thread) + CTimer::m_snTimeInMilliseconds;
	if (veh->m_nTimeTillWeNeedThisCar < time) {
		veh->m_nTimeTillWeNeedThisCar = time;
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_TIME_CHAR_IS_DEAD(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	int time = -1;
	if (!ped->IsAlive()) time = CTimer::m_snTimeInMilliseconds - ped->m_nDeathTime;
	CLEO_SetIntOpcodeParam(thread, time);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_TIME_CAR_IS_DEAD(CScriptThread* thread)
{
	CVehicle *veh = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	int time = -1;
	if (veh->m_fHealth <= 0.0f) time = CTimer::m_snTimeInMilliseconds - veh->m_nTimeWhenBlowedUp;
	CLEO_SetIntOpcodeParam(thread, time);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CHAR_IGNORE_DAMAGE_ANIMS(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	PedExtended &data = extData.Get(ped);
	bool set = CLEO_GetIntOpcodeParam(thread) == 1;
	if (&data != nullptr) {
		data.ignoreDamageAnims = set;
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_CHAR(CScriptThread* thread)
{
	bool bResult = false;
	CPed *pedA = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CPed *pedB = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	CVector posA = pedA->GetPosition();
	CVector posB = pedB->GetPosition();
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_CAR(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	CVector posA = ped->GetPosition();
	CVector posB = vehicle->GetPosition();
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_OBJECT(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	CVector posA = ped->GetPosition();
	CVector posB = object->GetPosition();
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CAR_DISTANCE_TO_OBJECT(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	CVector posA = vehicle->GetPosition();
	CVector posB = object->GetPosition();
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CAR_DISTANCE_TO_CAR(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicleA = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CVehicle *vehicleB = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	CVector posA = vehicleA->GetPosition();
	CVector posB = vehicleB->GetPosition();
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_OBJECT_DISTANCE_TO_OBJECT(CScriptThread* thread)
{
	bool bResult = false;
	CObject *objectA = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	CObject *objectB = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	float radius = CLEO_GetFloatOpcodeParam(thread);
	CVector posA = objectA->GetPosition();
	CVector posB = objectB->GetPosition();
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_COORDINATES(CScriptThread* thread)
{
	bool bResult = false;
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	CVector posA = ped->GetPosition();
	CVector posB;
	posB.x = CLEO_GetFloatOpcodeParam(thread);
	posB.y = CLEO_GetFloatOpcodeParam(thread);
	posB.z = CLEO_GetFloatOpcodeParam(thread);
	float radius = CLEO_GetFloatOpcodeParam(thread);
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_CAR_DISTANCE_TO_COORDINATES(CScriptThread* thread)
{
	bool bResult = false;
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CVector posA = vehicle->GetPosition();
	CVector posB;
	posB.x = CLEO_GetFloatOpcodeParam(thread);
	posB.y = CLEO_GetFloatOpcodeParam(thread);
	posB.z = CLEO_GetFloatOpcodeParam(thread);
	float radius = CLEO_GetFloatOpcodeParam(thread);
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_OBJECT_DISTANCE_TO_COORDINATES(CScriptThread* thread)
{
	bool bResult = false;
	CObject *object = CPools::GetObject(CLEO_GetIntOpcodeParam(thread));
	CVector posA = object->GetPosition();
	CVector posB;
	posB.x = CLEO_GetFloatOpcodeParam(thread);
	posB.y = CLEO_GetFloatOpcodeParam(thread);
	posB.z = CLEO_GetFloatOpcodeParam(thread);
	float radius = CLEO_GetFloatOpcodeParam(thread);
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LOCATE_ENTITY_DISTANCE_TO_ENTITY(CScriptThread* thread)
{
	bool bResult = false;
	CEntity *entityA = (CEntity *)CLEO_GetIntOpcodeParam(thread);
	CEntity *entityB = (CEntity *)CLEO_GetIntOpcodeParam(thread);
	CVector posA = entityA->GetPosition();
	CVector posB = entityB->GetPosition();
	float radius = CLEO_GetFloatOpcodeParam(thread);
	if (VectorSqrMagnitude(posA - posB) <= radius * radius) bResult = true;
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ENTITY_COORDINATES(CScriptThread* thread)
{
	CEntity *entity = (CEntity *)CLEO_GetIntOpcodeParam(thread);
	CVector pos = entity->GetPosition();
	CLEO_SetFloatOpcodeParam(thread, pos.x);
	CLEO_SetFloatOpcodeParam(thread, pos.y);
	CLEO_SetFloatOpcodeParam(thread, pos.z);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ENTITY_HEADING(CScriptThread* thread)
{
	CEntity *entity = (CEntity *)CLEO_GetIntOpcodeParam(thread);
	CVector pos = entity->GetPosition();
	CLEO_SetFloatOpcodeParam(thread, entity->GetHeading());
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CAR_OWNED_BY_PLAYER(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(vehicle->m_nVehicleFlags.bHasBeenOwnedByPlayer);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAR_OWNED_BY_PLAYER(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	vehicle->m_nVehicleFlags.bHasBeenOwnedByPlayer = (CLEO_GetIntOpcodeParam(thread) == 1);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MODEL_INFO(CScriptThread* thread)
{
	bool bResult = false;
	int model = CLEO_GetIntOpcodeParam(thread);
	CBaseModelInfo* modelInfo = nullptr;
	if (model >= 0) {
		modelInfo = CModelInfo::GetModelInfo(model);
	}
	if (modelInfo) bResult = true;
	CLEO_SetIntOpcodeParam(thread, (DWORD)modelInfo);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_ANIMGROUP(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, vehicle->m_pHandlingData->m_nAnimGroup);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_FEAR(CScriptThread* thread)
{
	CPed *ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	uintptr_t pedStats = (uintptr_t)ped->m_pStats;
	CLEO_SetIntOpcodeParam(thread, *(uint8_t*)(pedStats + 0x24));
	return OR_CONTINUE;
}

OpcodeResult WINAPI IS_CAR_CONVERTIBLE(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(vehicle->m_pHandlingData->m_nModelFlags.m_bConvertible); // for current plugin-sdk version need to add m_nModelFlags to union struct
	return OR_CONTINUE;
}
 
OpcodeResult WINAPI GET_CAR_VALUE(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetIntOpcodeParam(thread, vehicle->m_pHandlingData->m_nMonetaryValue);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAR_PEDALS(CScriptThread* thread)
{
	CVehicle *vehicle = CPools::GetVehicle(CLEO_GetIntOpcodeParam(thread));
	CLEO_SetFloatOpcodeParam(thread, vehicle->m_fGasPedal);
	CLEO_SetFloatOpcodeParam(thread, vehicle->m_fBreakPedal);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_LOADED_LIBRARY(CScriptThread* thread)
{
	LPSTR name = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	HMODULE module = GetModuleHandleA(name);
	CLEO_SetIntOpcodeParam(thread, (DWORD)module);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(module != nullptr);
	return OR_CONTINUE;
}

OpcodeResult WINAPI RETURN_TIMES(CScriptThread* thread)
{
	unsigned int times = CLEO_GetIntOpcodeParam(thread);
	int curSp = thread->sp;
	curSp -= times;
	if (curSp < 0) curSp = 0;
	thread->sp = curSp;
	thread->ip = thread->stack[curSp];
	return OR_CONTINUE;
}

OpcodeResult WINAPI READ_CLIPBOARD_DATA_TO(CScriptThread* thread)
{
	bool bResult = false;
	uintptr_t pointer = CLEO_GetIntOpcodeParam(thread);
	uint32_t size = CLEO_GetIntOpcodeParam(thread);
	if (OpenClipboard(RsGlobal.ps->window)) {
		if (IsClipboardFormatAvailable(CF_TEXT)) {
			HANDLE handle = GetClipboardData(CF_TEXT);
			if (handle) {

				LPWSTR lptstr = (LPWSTR)GlobalLock(handle);

				if (lptstr != NULL)
				{
					memcpy_s((void*)pointer, size, lptstr, size);
					bResult = true;
				}
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI WRITE_CLIPBOARD_DATA_FROM(CScriptThread* thread)
{
	bool bResult = false;
	uintptr_t pointer = CLEO_GetIntOpcodeParam(thread);
	uint32_t size = CLEO_GetIntOpcodeParam(thread);
	if (OpenClipboard(RsGlobal.ps->window)) {
		if (IsClipboardFormatAvailable(CF_TEXT)) {
			HLOCAL globalAlloc = GlobalAlloc(LMEM_MOVEABLE, size);

			LPWSTR lptstrCopy = (LPWSTR)GlobalLock(globalAlloc);
			if (lptstrCopy) {
				memcpy(lptstrCopy, (void*)pointer, size);
			}
			GlobalUnlock(globalAlloc);

			EmptyClipboard();

			HANDLE handle = SetClipboardData(CF_TEXT, globalAlloc);
			if (handle) bResult = true;
		}
		CloseClipboard();
	}
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(bResult);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_THIRD_PERSON_CAMERA_TARGET(CScriptThread* thread)
{
	float range = CLEO_GetFloatOpcodeParam(thread);
	float srcx = CLEO_GetFloatOpcodeParam(thread);
	float srcy = CLEO_GetFloatOpcodeParam(thread);
	float srcz = CLEO_GetFloatOpcodeParam(thread);

	CVector cameraPos = { 0.0f, 0.0f, 0.0f };
	CVector pointPos = { 0.0f, 0.0f, 0.0f };

	//CCamera::Find3rdPersonCamTargetVector(CCamera * this, float range, float srcx, float srcy, float srcz, CVector * pCamera, CVector * pPoint)
	plugin::CallMethod<0x514970, CCamera*, float, float, float, float, CVector*, CVector*>(&TheCamera, range, srcx, srcy, srcz, &cameraPos, &pointPos);

	CLEO_SetFloatOpcodeParam(thread, cameraPos.x);
	CLEO_SetFloatOpcodeParam(thread, cameraPos.y);
	CLEO_SetFloatOpcodeParam(thread, cameraPos.z);

	CLEO_SetFloatOpcodeParam(thread, pointPos.x);
	CLEO_SetFloatOpcodeParam(thread, pointPos.y);
	CLEO_SetFloatOpcodeParam(thread, pointPos.z);

	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_DISTANCE_MULTIPLIER(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, TheCamera.m_fLODDistMultiplier);
	CLEO_SetFloatOpcodeParam(thread, TheCamera.m_fGenerationDistMultiplier);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_ACTIVE_CAMERA_ROTATION(CScriptThread* thread)
{
	CMatrixLink* matrix = TheCamera.GetMatrix();

	if (matrix) {
		float x = matrix->right.x;
		float y = matrix->right.y;
		float z = matrix->right.z;
		float angleX = CGeneral::GetATanOfXY(z, sqrt(x * x + y * y)) * -57.295776f - 270.0f;
		while (angleX < 0.0f) angleX += 360.0f;

		float angleZ = CGeneral::GetATanOfXY(x, y) * -57.295776f - 180.0f;
		while (angleZ < 0.0f) angleZ += 360.0f;

		x = matrix->up.x;
		y = matrix->up.y;
		z = matrix->up.z;
		float angleY = CGeneral::GetATanOfXY(z, sqrt(x * x + y * y)) * -57.295776f - 270.0f;
		while (angleY < 0.0f) angleY += 360.0f;

		CLEO_SetFloatOpcodeParam(thread, angleX);
		CLEO_SetFloatOpcodeParam(thread, angleY);
		CLEO_SetFloatOpcodeParam(thread, angleZ);
	}
	else {
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
		CLEO_SetFloatOpcodeParam(thread, 0.0f);
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CLOSEST_WATER_DISTANCE(CScriptThread* thread)
{
	CLEO_SetFloatOpcodeParam(thread, TheCamera.m_fDistanceToWater);
	CLEO_SetFloatOpcodeParam(thread, TheCamera.m_fHeightOfNearestWater);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAMERA_STRUCT(CScriptThread* thread)
{
	CLEO_SetIntOpcodeParam(thread, (DWORD)&TheCamera);
	CLEO_SetIntOpcodeParam(thread, (DWORD)&TheCamera.m_aCams[TheCamera.m_nActiveCam]);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CAMERA_ROTATION_INPUT_VALUES(CScriptThread* thread)
{
	CCam* cam = &TheCamera.m_aCams[TheCamera.m_nActiveCam];
	CLEO_SetFloatOpcodeParam(thread, cam->m_fHorizontalAngle * -57.295776f);
	CLEO_SetFloatOpcodeParam(thread, cam->m_fVerticalAngle * 57.295776f);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_CAMERA_ROTATION_INPUT_VALUES(CScriptThread* thread)
{
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	CCam* cam = &TheCamera.m_aCams[TheCamera.m_nActiveCam];
	cam->m_fHorizontalAngle = (x * -0.01745329252f);
	cam->m_fVerticalAngle = (y * 0.01745329252f);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_ON_MISSION(CScriptThread* thread)
{
	*(scriptSpace + CTheScripts::OnAMissionFlag) = CLEO_GetIntOpcodeParam(thread);;
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MODEL_NAME_POINTER(CScriptThread* thread)
{
	unsigned int id = CLEO_GetIntOpcodeParam(thread);
	if (id >= 0 && id < maxDffFiles)
	{
		CLEO_SetIntOpcodeParam(thread, (DWORD)&modelNames[id * 32]);
	}
	else {
		CLEO_SetIntOpcodeParam(thread, 0);
	}
	return OR_CONTINUE;
}
