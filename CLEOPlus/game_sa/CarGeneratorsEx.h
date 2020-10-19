#pragma once

#include "CCarGenerator.h"

class CCarGenerator_extended : public CCarGenerator
{
public:
	uint16_t lastRandomizedID;
	uint16_t exIplFile;
	CVector exPos;

	//// version of 0x30 bytes /////
};