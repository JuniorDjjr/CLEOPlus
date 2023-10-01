#include "OpcodesCommon.h"
#include "..\injector\assembly.hpp"
#include "CRadar.h"
#include "RadarBlip.h"
#include "CMenuManager.h"
#include "CTheZones.h"
#include <set>

using namespace std;
using namespace plugin;
using namespace injector;

extern bool gtavhudInstalled;
extern float *defaultBlipSize;
extern bool blipsAlwaysRevealed;

namespace RadarBlip
{
	class CRadarBlipCLEO
	{
	public:
		CEntity *entity;
		CVector2D worldPos;
		CRGBA color;
		CSprite2d *sprite;
		int spriteId;
		float angle;
		bool shortDistance;

		CRadarBlipCLEO(int _spriteId, CSprite2d *_sprite, float x, float y, bool _shortDistance, int r, int g, int b, int a)
		{
			entity = nullptr;
			spriteId = _spriteId;
			sprite = _sprite;
			worldPos.x = x;
			worldPos.y = y;
			shortDistance = _shortDistance;
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
			angle = 0.0f;
		}
	};

	std::vector<CRadarBlipCLEO*> blips;

	void MyDrawRadarSprite(CRadarBlipCLEO *blip, float x, float y)
	{
		float blipSize = *defaultBlipSize;
		if (FrontEndMenuManager.m_bDrawRadarOrMap)
		{
			x = RsGlobal.maximumWidth * 0.0015625f * x;
			y = RsGlobal.maximumHeight * 0.002232143f * y;
			CRadar::LimitToMap(&x, &y);
		}
		else {
			if (gtavhudInstalled) blipSize = 6.0f;
		}

		float width = (RsGlobal.maximumWidth * *ARwidthFromWF * blipSize);
		float height = (RsGlobal.maximumHeight * *ARheightFromWF * blipSize);

		int spriteIdForDisplay = (blip->spriteId != -1) ? blip->spriteId : RADAR_SPRITE_PROPERTYG;
		int spriteIdForLegend = (blip->spriteId != -1) ? blip->spriteId : RADAR_SPRITE_NONE;

		if (CRadar::DisplayThisBlip(spriteIdForDisplay, -99))
		{
			CRect rect;
			rect.left = x - width;
			rect.top = y - height;
			rect.right = width + x;
			rect.bottom = height + y;
			blip->sprite->Draw(rect, blip->color);
			if (spriteIdForLegend != RADAR_SPRITE_NONE) CRadar::AddBlipToLegendList(0, spriteIdForLegend);
		}
	}

	bool MyHasThisBlipBeenRevealed(CRadarBlipCLEO *blip)
	{
		if (
			!FrontEndMenuManager.m_bDrawRadarOrMap ||
			*(int*)0xBA372C >= totalZonesToReveal ||
			CTheZones::GetCurrentZoneLockedOrUnlocked(blip->worldPos.x, blip->worldPos.y) != 0
			)
		{
			return true;
		}
		return false;
	}

	void Patch()
	{
		injector::MakeInline<0x58810C, 0x58810C + 8>([](injector::reg_pack& regs)
		{
			*(uint32_t*)(regs.esp + 0x64) = 0; //mov dword ptr [esp+64h], 0
			if (blips.size() > 0)
			{
				for (CRadarBlipCLEO *blip : blips)
				{
					if (blip->entity)
					{
						CVector entityPos = blip->entity->GetPosition();
						blip->worldPos.x = entityPos.x;
						blip->worldPos.y = entityPos.y;
					}
					CVector2D radarPoint;
					CVector2D screenPos;
					CRadar::TransformRealWorldPointToRadarSpace(radarPoint, blip->worldPos);
					float distance = CRadar::LimitRadarPoint(radarPoint);
					if (!blip->shortDistance || distance <= 1.0f || FrontEndMenuManager.m_bDrawRadarOrMap)
					{
						if (MyHasThisBlipBeenRevealed(blip) || blipsAlwaysRevealed)
						{
							CRadar::TransformRadarPointToScreenSpace(screenPos, radarPoint);
							MyDrawRadarSprite(blip, screenPos.x, screenPos.y);
						}
					}
				}
			}
		});

	}

	void Clear()
	{
		for (CRadarBlipCLEO* blip : blips)
		{
			delete blip;
		}
		blips.clear();
	}
}

using namespace RadarBlip;

OpcodeResult WINAPI ADD_CLEO_BLIP(CScriptThread* thread)
{
	int spriteId = CLEO_GetIntOpcodeParam(thread);
	unsigned int spriteIdUnsigned = (unsigned int)spriteId;
	CSprite2d *sprite = new CSprite2d();
	if (spriteId < 0 && spriteId > -10000) {
		sprite->m_pTexture = (RwTexture*)CLEO_GetScriptTextureById(thread, (spriteId * -1));
		spriteId = -1;
	}
	else {
		if (spriteIdUnsigned > 0x00010000) {
			sprite->m_pTexture = (RwTexture*)spriteIdUnsigned;
			spriteId = -1;
		}
		else {
			sprite->m_pTexture = radarBlipSprites[spriteIdUnsigned].m_pTexture;
		}
	}
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	int shortDistance = CLEO_GetIntOpcodeParam(thread);
	int r = CLEO_GetIntOpcodeParam(thread);
	int g = CLEO_GetIntOpcodeParam(thread);
	int b = CLEO_GetIntOpcodeParam(thread);
	int a = CLEO_GetIntOpcodeParam(thread);
	CRadarBlipCLEO *blip = new CRadarBlipCLEO(spriteId, sprite, x, y, shortDistance, r, g, b, a);
	blips.push_back(blip);
	CLEO_SetIntOpcodeParam(thread, (DWORD)blip);
	return OR_CONTINUE;
}

OpcodeResult WINAPI REMOVE_CLEO_BLIP(CScriptThread* thread)
{
	CRadarBlipCLEO *blip = (CRadarBlipCLEO *)CLEO_GetIntOpcodeParam(thread);
	for (std::vector<CRadarBlipCLEO*>::iterator iter = blips.begin(); iter != blips.end(); ++iter)
	{
		if (*iter == blip)
		{
			delete blip;
			blips.erase(iter);
			break;
		}
	}
	return OR_CONTINUE;
}

