#include "OpcodesCommon.h"

OpcodeResult WINAPI GET_CHAR_BONE_MATRIX(CScriptThread* thread)
{
	CPed* ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	unsigned int bone = CLEO_GetIntOpcodeParam(thread);

	if (!ped->m_pRwObject)
	{
		CLEO_SetIntOpcodeParam(thread, 0);
		reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(false);
		return OR_CONTINUE;
	}
	ped->UpdateRpHAnim();
	RwMatrix* matrix = nullptr;
	RpHAnimHierarchy* hierarchy = GetAnimHierarchyFromSkinClump((RpClump*)ped->m_pRwObject);
	if (!hierarchy)
		matrix = (RwMatrix*)ped->GetMatrix();
	else
		matrix = &RpHAnimHierarchyGetMatrixArray(hierarchy)[RpHAnimIDGetIndex(hierarchy, bone)];

	CLEO_SetIntOpcodeParam(thread, (DWORD)matrix);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag((matrix != nullptr));
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_CHAR_BONE(CScriptThread* thread)
{
	CPed* ped = CPools::GetPed(CLEO_GetIntOpcodeParam(thread));
	//originally NewOpcodes also supports string as bone name, but isn't a good practice for many reasons, enum is much better
	int boneId = CLEO_GetIntOpcodeParam(thread);
	auto bone = &RpClumpGetAnimBlendClumpData((RpClump*)ped->m_pRwObject)->m_pFrames[boneId];
	CLEO_SetIntOpcodeParam(thread, (DWORD)bone);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag((bone != nullptr));
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_BONE_OFFSET_VECTOR(CScriptThread* thread)
{
	AnimBlendFrameData* bone = (AnimBlendFrameData*)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)&bone->m_vecOffset);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_BONE_QUAT(CScriptThread* thread)
{
	AnimBlendFrameData* bone = (AnimBlendFrameData*)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, (DWORD)bone->m_pIFrame); //IFrame->m_qOrientation (offset 0x0)
	return OR_CONTINUE;
}
