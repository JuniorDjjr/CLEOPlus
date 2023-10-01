#pragma once

class SpecialModel
{
public:
	RpClump* clump;
	RpAtomic* atomic;
	int txdIndex;
	int refCount;
	int loadMode;

	SpecialModel(RpClump* _clump, RpAtomic* _atomic, int _txdIndex, int _loadMode);
	bool Remove();
};
