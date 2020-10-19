#include "OpcodesCommon.h"

#include "CPlayerPed.h"
#include "CPlayerInfo.h"
#include "CPlayerData.h"
#include "CPedGroups.h"
#include "CWorld.h"
#include "CGame.h"
#include "CAnimManager.h"
#include ".\..\injector\calling.hpp"
#include "CPedIntelligence.h"
#include "extensions/ScriptCommands.h"
#include "CTaskComplexFacial.h"
#include "CTaskSimpleStandStill.h"
#include "CTaskSimplePlayerOnFoot.h"
#include "CCamera.h"

using namespace plugin;
using namespace injector;

CPlayerData *backup_PlayerData;
bool g_FixTwoPlayersSeparatedCars;

OpcodeResult WINAPI SET_CHAR_SECOND_PLAYER(CScriptThread* thread) {

	CPlayerPed *ped = (CPlayerPed *)CPools::ms_pPedPool->GetAtRef(CLEO_GetIntOpcodeParam(thread));
	int enable_camera = CLEO_GetIntOpcodeParam(thread);
	int separated_cars = CLEO_GetIntOpcodeParam(thread);

	CPlayerPed *pedTest = CWorld::Players[1].m_pPed;
	if (!pedTest) {

		ped->m_nPedType = PED_TYPE_PLAYER1;


		//MakeNOP(0x60D5CB, 11, true);

		//CPlayerPed *ped = (CPlayerPed *)CPed::operator new(1956);
		//((void(__thiscall *)(CPed*, ePedType))0x5E8030)(ped, ePedType::PED_TYPE_PLAYER1);

		//plugin::CallMethod<0x60D5B0, CPlayerPed *, int, bool>(ped, 1, false);


		// ----- Custom CPlayerPed::CPlayerPed
		CPlayerData *playerData = &CWorld::Players[1].m_PlayerData;

		*(uint32_t*)ped = 0x86D168;

		backup_PlayerData = ped->m_pPlayerData;

		ped->m_pPlayerData = playerData;

		//void __thiscall CPlayerPedData::AllocateData(CPlayerData *this)
		plugin::CallMethod<0x571880, CPlayerData *>(playerData); //CPlayerPedData::AllocateData

		//ped->SetModelIndex(0);

		ped->SetInitialState(false);

		//CEntity *obj = ped->m_pTargetedObject;
		//if (obj) obj->CleanUpOldReference(&obj);
		//ped->m_pTargetedObject = 0;
		ped->ClearWeaponTarget();

		//ped->SetPedState(PEDSTATE_IDLE);

		int playidles_animIndex = CAnimManager::GetAnimationBlockIndex("playidles");
		WriteMemory(0xC0BC10, playidles_animIndex, false);

		//group
		//playerData->m_nPlayerGroup = CPedGroups::AddGroup();

		ped->m_pPlayerTargettedPed = 0;
		ped->field_7A0 = 0;

		ped->m_pIntelligence->m_fDmRadius = 30.0;
		ped->m_pIntelligence->m_nDmNumPedsToScan = 2;

		uint32_t *flags = (uint32_t*)(&ped->m_nPedFlags);
		flags[3] ^= (flags[3] ^ (false << 21)) & 0x200000;


		// ----- Post setup
		CWorld::Players[1].m_pPed = ped;
		ped->m_nPedType = PED_TYPE_PLAYER2;
		//ped->SetOrientation(0.0, 0.0, 0.0);
		//CWorld::Add(CWorld::Players[1].m_pPed);

		ped->m_nWeaponAccuracy = 100;
		CWorld::Players[1].m_nPlayerState = PLAYERSTATE_PLAYING;


		// ---- Post creation
		//ped->SetCharCreatedBy(2);
		//CWorld::Remove(CWorld::Players[1].m_pPed);
		//CWorld::Add(CWorld::Players[1].m_pPed);

		//ped->m_pIntelligence->m_TaskMgr.SetTask(new CTaskSimplePlayerOnFoot(), 4, false);
	}

	if (enable_camera) {
		TheCamera.StartCooperativeCamMode();
	}

	if (separated_cars) {
		*(uint8_t*)0x96A8B3 = true;
		g_FixTwoPlayersSeparatedCars = true;
	}

	return OR_CONTINUE;
}

OpcodeResult WINAPI DISABLE_SECOND_PLAYER(CScriptThread* thread) {
	int restore_camera = CLEO_GetIntOpcodeParam(thread);

	CPlayerPed *ped = CWorld::Players[1].m_pPed;
	if (ped) {

		*(uint32_t*)ped = 0x86C358;

		CWorld::Players[1].m_pPed = nullptr;

		//delete &CWorld::Players[1];
		//CWorld::Players[1].~Player
		//reinterpret_cast<void(__thiscall *)(CPlayerInfo *)>(0x45B110)(&CWorld::Players[1]);
		//plugin::CallMethod<0x45B110, CPlayerInfo *, int>(&CWorld::Players[1], 0); //CPlayerInfo::Clear

		ped->m_pPlayerTargettedPed = 0;
		ped->field_7A0 = 0;

		ped->m_pPlayerData = nullptr;

		ped->m_pIntelligence->FlushIntelligence();
		plugin::CallMethod<0x607140, CPedIntelligence *, CPed *>(ped->m_pIntelligence, ped);
		ped->m_pIntelligence->m_TaskMgr.SetTaskSecondary(new CTaskComplexFacial, 3);
		ped->m_pIntelligence->m_TaskMgr.SetTask(new CTaskSimpleStandStill(0, 1, 0, 8.0), 0, true);

		injector::thiscall<void(CPlayerInfo*)>::call<0x45B110>(&CWorld::Players[1]);

		//plugin::CallMethod<0x607300, CPedIntelligence *>(ped->m_pIntelligence);
		//CPedIntelligence *intelligence = new CPedIntelligence;
		//5E849A

		ped->m_nPedType = PED_TYPE_MISSION1;

		if (restore_camera) {
			TheCamera.RestoreWithJumpCut();
		}
		*(uint8_t*)0x96A8B3 = false; // disable separated cars by default
		g_FixTwoPlayersSeparatedCars = false;
	}

	return OR_CONTINUE;
}

OpcodeResult WINAPI FIX_TWO_PLAYERS_SEPARATED_CARS(CScriptThread* thread) {
	bool fix = CLEO_GetIntOpcodeParam(thread);
	if (g_FixTwoPlayersSeparatedCars && fix) *(uint8_t*)0x96A8B3 = false; // disable separated cars by default if fix was enabled
	g_FixTwoPlayersSeparatedCars = fix;
	return OR_CONTINUE;
}

void PatchCoop() {
	Events::processScriptsEvent.after += [] {
		if (g_FixTwoPlayersSeparatedCars) {
			if (CWorld::Players[0].m_pPed && CWorld::Players[1].m_pPed) {
				CVector pos1 = CWorld::Players[0].m_pPed->GetPosition();
				CVector pos2 = CWorld::Players[1].m_pPed->GetPosition();
				CVector distVector;
				distVector.x = pos1.x - pos2.x;
				distVector.y = pos1.y - pos2.y;
				distVector.z = pos1.z - pos2.z;
				float dist = distVector.Magnitude();
				if (dist < 5.0f) {
					if (CWorld::Players[0].m_pPed->m_pVehicle || CWorld::Players[1].m_pPed->m_pVehicle)
					{
						if (CWorld::Players[0].m_pPed->m_pVehicle && CWorld::Players[1].m_pPed->m_pVehicle && (CWorld::Players[0].m_pPed->m_pVehicle == CWorld::Players[1].m_pPed->m_pVehicle)) {
							*(uint8_t*)0x96A8B3 = false;
						}
						else {
							if (CWorld::Players[0].m_pPed->m_pVehicle && !CWorld::Players[1].m_pPed->m_pVehicle)
							{
								if (CWorld::Players[0].m_pPed->m_pVehicle->m_nNumPassengers < (CWorld::Players[0].m_pPed->m_pVehicle->m_nMaxPassengers - 1))
								{
									*(uint8_t*)0x96A8B3 = false;
								}
								else {
									*(uint8_t*)0x96A8B3 = true;
								}
							}
							if (CWorld::Players[1].m_pPed->m_pVehicle && !CWorld::Players[0].m_pPed->m_pVehicle)
							{
								if (CWorld::Players[1].m_pPed->m_pVehicle->m_nNumPassengers < (CWorld::Players[1].m_pPed->m_pVehicle->m_nMaxPassengers - 1))
								{
									*(uint8_t*)0x96A8B3 = false;
								}
								else {
									*(uint8_t*)0x96A8B3 = true;
								}
							}
						}
					}
					else {
						*(uint8_t*)0x96A8B3 = true;
					}
				}
				else {
					*(uint8_t*)0x96A8B3 = true;
				}
			}
		}
	};

}