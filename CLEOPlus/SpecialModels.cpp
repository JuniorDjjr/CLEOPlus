#include "OpcodesCommon.h"
#include "CModelInfo.h"
#include "CPedModelInfo.h"
#include "CStreaming.h"
#include "CDirectory.h"
#include "CTxdStore.h"
#include "SpecialModels.h"

SpecialModel::SpecialModel(RpClump* _clump, RpAtomic* _atomic, int _loadMode)
{
	clump = _clump;
	atomic = _atomic;
	loadMode = _loadMode;
}

OpcodeResult WINAPI LOAD_SPECIAL_MODEL(CScriptThread* thread)
{
	string nameDff = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);
	string nameTxd = CLEO_ReadStringPointerOpcodeParam(thread, bufferA, 128);

	nameDff.append(".dff");
	nameTxd.append(".txd");

	int index = CTxdStore::FindTxdSlot(&nameTxd[0]);
	if (index != -1) {
		if (plugin::CallAndReturn<bool, 0x407800, int>(index + 20000)) {
			CStreaming::RequestModel(index + 20000, MISSION_REQUIRED);
			CStreaming::LoadAllRequestedModels(false);
		}
	}
	else {
		index = CTxdStore::AddTxdSlot(&nameTxd[0]);
		if (CTxdStore::LoadTxd(index, &nameTxd[0])) {
			CTxdStore::AddRef(index);
		}
	}

	CTxdStore::SetCurrentTxd(index);

	SpecialModel* specialModel = nullptr;

	RwStream* stream = RwStreamOpen(RwStreamType::rwSTREAMFILENAME, RwStreamAccessType::rwSTREAMREAD, &nameDff[0]);
	if (stream)
	{
		if (RwStreamFindChunk(stream, 0x10, 0, 0))
		{
			RpClump *clump = RpClumpStreamRead(stream);
			if (clump)
			{
				RpAtomic *atomic = GetFirstAtomic(clump);
				//atomic->renderCallBack = AtomicDefaultRenderCallBack;

				if (atomic)
				{
					specialModel = new SpecialModel(clump, atomic, 1);
				}
			}
		}
		RwStreamClose(stream, 0);
	}

	CTxdStore::PopCurrentTxd();

	CLEO_SetIntOpcodeParam(thread, (DWORD)specialModel);
	reinterpret_cast<CRunningScript*>(thread)->UpdateCompareFlag(specialModel);
	return OR_CONTINUE;
}
