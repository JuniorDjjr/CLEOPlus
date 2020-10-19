#pragma once

#include "game_sa\CFont.h"
#include "../Drawing.h"

struct tFontPrint
{
	char m_text[128];
	float m_fX, m_fY;
	float m_fWidth, m_fHeight;
	unsigned char m_nFixAR, m_nStyle, m_nProp, m_nOutline, m_nShadow;
	unsigned char m_nJustify, m_nAlign, m_nBackground;
	CRGBA m_Color, m_DropColor, m_BackColor;
	float m_fWrap;
};

class TextDrawer
{
public:
	std::vector<tFontPrint*> m_aPrints;
	void DrawPrints();
	void ClearAll();
};

extern TextDrawer textDrawer[DrawEvent::TOTAL_DRAW_EVENT];