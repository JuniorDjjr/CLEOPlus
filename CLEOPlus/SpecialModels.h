#pragma once

class SpecialModel
{
public:
	RpClump* clump;
	RpAtomic* atomic;
	int loadMode;

	SpecialModel(RpClump* _clump, RpAtomic* _atomic, int _loadMode);
};
