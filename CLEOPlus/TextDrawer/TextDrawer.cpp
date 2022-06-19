#include "CMessages.h"
#include "TextDrawer.h"
#include "../Screen.h"

TextDrawer textDrawer[DrawEvent::TOTAL_DRAW_EVENT];

void TextDrawer::DrawPrints()
{
	char finalString[400];
	for (auto print : m_aPrints)
	{
		float sizX = print->m_fWidth;
		float sizY = print->m_fHeight;

		if (print->m_nFixAR) {
			FixAspectRatio(&sizX, &sizY);
		}

		float magicResolutionWidth = RsGlobal.maximumWidth * 0.0015625f;
		float magicResolutionHeight = RsGlobal.maximumHeight * 0.002232143f;

		CFont::SetScale(sizX * magicResolutionWidth, sizY * magicResolutionHeight);
		CFont::SetFontStyle(print->m_nStyle);
		CFont::SetProportional(print->m_nProp);
		CFont::SetJustify(print->m_nJustify);
		CFont::SetOrientation((eFontAlignment)print->m_nAlign);
		if(print->m_nOutline || print->m_nShadow)
		{
			if(print->m_nOutline)
				CFont::SetEdge(print->m_nOutline);
			else
				CFont::SetDropShadowPosition(print->m_nShadow);
			CFont::SetDropColor(print->m_DropColor);
		}
		else
		{
			CFont::SetEdge(0);
			CFont::SetDropShadowPosition(0);
		}
		if(print->m_nBackground)
		{
			CFont::SetBackground(print->m_nBackground, false);
			CFont::SetBackgroundColor(print->m_BackColor);
		}
		else
			CFont::SetBackground(false, false);
		CFont::SetColor(print->m_Color);
		switch(print->m_nAlign)
		{
			case ALIGN_LEFT:
				CFont::SetWrapx(print->m_fWrap * magicResolutionWidth); break;
			case ALIGN_CENTER:
				CFont::SetCentreSize(print->m_fWrap * magicResolutionWidth); break;
			case ALIGN_RIGHT:
				CFont::SetRightJustifyWrap(print->m_fWrap * magicResolutionWidth); break;
		}
		CMessages::InsertNumberInString(print->m_text, -1, -1, -1, -1, -1, -1, finalString); // this is required to use InsertPlayerControlKeysInString, idkw
		CMessages::InsertPlayerControlKeysInString(finalString);
		CFont::PrintString(print->m_fX * magicResolutionWidth, print->m_fY * magicResolutionHeight, finalString);
	}

	// clear all
	ClearAll();
}

void TextDrawer::ClearAll() {
	unsigned int size = m_aPrints.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		delete m_aPrints[i];
	}
	m_aPrints.clear();
}