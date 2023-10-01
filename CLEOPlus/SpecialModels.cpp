#include "OpcodesCommon.h"
#include "CModelInfo.h"
#include "CPedModelInfo.h"
#include "CStreaming.h"
#include "CDirectory.h"
#include "CTxdStore.h"
#include "SpecialModels.h"

SpecialModel::SpecialModel(RpClump* _clump, RpAtomic* _atomic, int _txdIndex, int _loadMode)
{
	clump = _clump;
	atomic = _atomic;
	txdIndex = _txdIndex;
	loadMode = _loadMode;
	refCount = 1;
}

bool SpecialModel::Remove()
{
	refCount -= 1;
	if (refCount <= 0) {
		RpAtomicDestroy(atomic);
		atomic = nullptr;
		RpClumpDestroy(clump);
		clump = nullptr;
		CTxdStore::RemoveRef(txdIndex);
		return true;
	}
	return false;
}

OpcodeResult WINAPI LOAD_SPECIAL_MODEL(CScriptThread* thread)
{
	string nameDff = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	string nameTxd = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);

	SpecialModel* specialModel = nullptr;

	nameDff.append(".dff");
	nameTxd.append(".txd");

	int index = CTxdStore::FindTxdSlot(&nameTxd[0]);
	if (index != -1) {
		//if (plugin::CallAndReturn<bool, 0x407800, int>(index + 20000)) {
			//CStreaming::RequestTxdModel(index, MISSION_REQUIRED);
			//CStreaming::LoadAllRequestedModels(false);
		//}
	}
	else {
		index = CTxdStore::AddTxdSlot(&nameTxd[0]);
	}

	if (CTxdStore::LoadTxd(index, &nameTxd[0])) {
		CTxdStore::AddRef(index);

		CTxdStore::SetCurrentTxd(index);

		RwStream* stream = RwStreamOpen(RwStreamType::rwSTREAMFILENAME, RwStreamAccessType::rwSTREAMREAD, &nameDff[0]);
		if (stream)
		{
			if (RwStreamFindChunk(stream, 0x10, 0, 0))
			{
				RpClump* clump = RpClumpStreamRead(stream);
				if (clump)
				{
					RpAtomic* atomic = GetFirstAtomic(clump);
					//atomic->renderCallBack = AtomicDefaultRenderCallBack;

					if (atomic)
					{
						specialModel = new SpecialModel(clump, atomic, index, 1);
					}
				}
			}
			RwStreamClose(stream, 0);
		}

		CTxdStore::PopCurrentTxd();
	}

	CLEO_SetIntOpcodeParam(thread, (DWORD)specialModel);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(specialModel);
	return OR_CONTINUE;
}

OpcodeResult WINAPI REMOVE_SPECIAL_MODEL(CScriptThread* thread)
{
	SpecialModel* specialModel = reinterpret_cast<SpecialModel*>(CLEO_GetIntOpcodeParam(thread));

	if (specialModel) {
		if (specialModel->Remove()) {
			delete specialModel;
		}
	}
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_SPECIAL_MODEL_DATA(CScriptThread* thread)
{
	SpecialModel* specialModel = reinterpret_cast<SpecialModel*>(CLEO_GetIntOpcodeParam(thread));

	if (specialModel) {
		CLEO_SetIntOpcodeParam(thread, (DWORD)specialModel->clump);
		CLEO_SetIntOpcodeParam(thread, (DWORD)specialModel->atomic);
		CLEO_SetIntOpcodeParam(thread, (DWORD)specialModel->txdIndex);
	}
	else {
		CLEO_SetIntOpcodeParam(thread, (DWORD)0);
		CLEO_SetIntOpcodeParam(thread, (DWORD)0);
		CLEO_SetIntOpcodeParam(thread, (DWORD)-1);
	}
	return OR_CONTINUE;
}
