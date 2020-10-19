#pragma once
#include "PluginBase.h"
#include "CEntity.h"

class CPedDamageResponseCalculator {
public:
	CEntity *m_pDamager;
	float m_fDamageFactor;
	uint32_t m_pedPieceType;
	uint32_t m_weaponType;
	uint32_t m_bUnk;
};
VALIDATE_SIZE(CPedDamageResponseCalculator, 0x14);
