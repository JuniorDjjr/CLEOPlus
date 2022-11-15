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
	return OR_CONTINUE;
}
