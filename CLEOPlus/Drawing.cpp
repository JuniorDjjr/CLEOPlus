#include "OpcodesCommon.h"
#include "Screen.h"
#include "CHud.h"
#include "Drawing.h"
#include "CTheScripts.h"
#include "TextDrawer/TextDrawer.h"

using namespace std;

class MySprite
{
public:
	RwTexture *texture;
	float cornerA_X;
	float cornerA_Y;
	float cornerB_X;
	float cornerB_Y;
	float angle;
	float depth;
	bool fixAR;
	CRGBA rgba;
	int maskTrisCount;
	float *maskTrisArray;

	MySprite(RwTexture *newTexture, float _cornerA_X, float _cornerA_Y, float _cornerB_X, float _cornerB_Y, float _angle, float _depth, int _fixAR, int _maskTrisCount, float *_maskTrisArray, int red, int green, int blue, int alpha)
	{
		texture = newTexture;
		cornerA_X = _cornerA_X;
		cornerA_Y = _cornerA_Y;
		cornerB_X = _cornerB_X;
		cornerB_Y = _cornerB_Y;
		angle = _angle;
		depth = _depth;
		fixAR = _fixAR;
		maskTrisCount = _maskTrisCount;
		maskTrisArray = _maskTrisArray;
		rgba.r = red;
		rgba.g = green;
		rgba.b = blue;
		rgba.a = alpha;
	}

	~MySprite() 
	{
		if (maskTrisCount > 0 && maskTrisArray) free(maskTrisArray);
	}
};

class MyMask
{
public:
	CVector2D pos;
	CVector2D size;
	CRGBA rgba;

	MyMask(float x, float y, float sizeX, float sizeY)
	{
		pos.x = x;
		pos.y = y;
		size.x = sizeX;
		size.y = sizeY;
	}
};

vector<MySprite*> sprites[DrawEvent::TOTAL_DRAW_EVENT];

void ClearMySprites(vector<MySprite*> &spritesList)
{
	if (spritesList.size() > 0)
	{
		for (auto mySprite : spritesList)
		{
			delete mySprite;
		}
		spritesList.clear();
	}
}

float texCoors[8];
float verts[16];
float verts2[16];
RwD3D9Vertex maskVertexBuffer[128];

// Converted from 0x007279B0
void MySetMaskVertices(int num, RwV2d *a2, float z)
{
	int v3, v4;
	v3 = 0;
	if (num > 0)
	{
		v4 = (int)&maskVertexBuffer->y;
		do
		{
			*(float*)(v4 - 4) = a2[v3].x;
			*(float*)v4 = a2[v3].y;
			*(float*)(v4 + 4) = z;
			*(float*)(v4 + 8) = CSprite2d::RecipNearClip;
			*(float*)(v4 + 12) = 0;
			++v3;
			v4 += 28;
		} while (v3 < num);
	}
}

void DrawMySprites(vector<MySprite*> &spritesList)
{
	if (spritesList.size() > 0)
	{
		texCoors[0] = 0.0f;
		texCoors[1] = 0.0f;
		texCoors[2] = 1.0f;
		texCoors[3] = 0.0f;
		texCoors[4] = 0.0f;
		texCoors[5] = 1.0f;
		texCoors[6] = 1.0f;
		texCoors[7] = 1.0f;

		for (auto mySprite : spritesList)
		{
			RwRaster *raster = nullptr;
			if (mySprite->texture) raster = mySprite->texture->raster;

			if (mySprite->maskTrisCount > 0)
			{
				if (mySprite->maskTrisArray != nullptr) {
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, 0);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)2);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)1); //D3DSHADE_FLAT
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, 0);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)1);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)8);

					MySetMaskVertices(mySprite->maskTrisCount, (RwV2d *)mySprite->maskTrisArray, CSprite2d::NearScreenZ + mySprite->depth);
					RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
					RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRISTRIP, maskVertexBuffer, mySprite->maskTrisCount);
				}

				// Sprite render state compatible with mask
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, 0);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5u);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6u);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)2u);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)1); //D3DSHADE_FLAT
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)1); //rwRENDERSTATETEXTURERASTER
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 0);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)3); //D3DTADDRESS_CLAMP
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, 0);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)5);
			} 
			else
			{
				// Sprite render state NOT compatible with mask
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, 0);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5u);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6u);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)2u);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)1); //D3DSHADE_FLAT
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 0);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)2); //D3DTADDRESS_CLAMP
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, 0);
			}

			float v8 = (mySprite->cornerB_X + mySprite->cornerA_X) * 0.5f;
			float v9 = (mySprite->cornerB_Y + mySprite->cornerA_Y) * 0.5f;
			float v10 = v8 - mySprite->cornerA_X;
			float v11 = v9 - mySprite->cornerA_Y;
			float v12 = cos(mySprite->angle);
			float v13 = sin(mySprite->angle);
			float v14 = v12 * v11;
			float v15 = v13 * v10;
			float v16 = v12 * v10;
			float v17 = v13 * v11;
			float v18 = -v16;
			verts2[7] = v15 + v14 + v9;
			verts2[6] = v16 - v17 + v8;
			verts2[5] = v14 - v15 + v9;
			verts2[4] = v18 - v17 + v8;
			verts2[3] = v15 - v14 + v9;
			verts2[2] = v17 + v16 + v8;
			verts2[1] = -v15 - v14 + v9;
			verts2[0] = v18 + v17 + v8;

			if (mySprite->maskTrisCount > 0)
			{
				CSprite2d::NearScreenZ += mySprite->depth;
				CSprite2d::SetVertices(4, verts2, texCoors, mySprite->rgba);
				CSprite2d::NearScreenZ -= mySprite->depth;

				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
				RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRISTRIP, CSprite2d::maVertices, 4);
			}
			else
			{
				CSprite2d::SetVertices(verts2[0], verts2[1], verts2[2], verts2[3], verts2[4], verts2[5], verts2[6], verts2[7], mySprite->rgba, mySprite->rgba, mySprite->rgba, mySprite->rgba);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
				RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
			}

		}
	}
}


//DRAW_TEXTURE_PLUS 0 0 100.0 100.0 50.0 50.0 90.0 TRUE 0 0 255 255 255 255
// // 0E1E=15,draw_texture_plus %1d% event %2d% pos %3d% %4d% size %5d% %6d% angle %7d% depth %8d% fix_aspect_ratio %9d% maskTrisCount %10d% maskTrisArray %11d% rgba %12d% %13d% %14d% %15d% 
OpcodeResult WINAPI DRAW_TEXTURE_PLUS(CScriptThread* thread)
{
	RwTexture *texture = (RwTexture*)CLEO_GetIntOpcodeParam(thread);
	int drawEvent = CLEO_GetIntOpcodeParam(thread);
	float posX = CLEO_GetFloatOpcodeParam(thread);
	float posY = CLEO_GetFloatOpcodeParam(thread);
	float sizX = CLEO_GetFloatOpcodeParam(thread);
	float sizY = CLEO_GetFloatOpcodeParam(thread);
	float angle = CLEO_GetFloatOpcodeParam(thread);
	float depth = CLEO_GetFloatOpcodeParam(thread);
	int fixAspectRatio = CLEO_GetIntOpcodeParam(thread);
	int maskTrisCount = CLEO_GetIntOpcodeParam(thread);
	float *maskTrisArray = (float *)CLEO_GetIntOpcodeParam(thread);
	int red = CLEO_GetIntOpcodeParam(thread);
	int green = CLEO_GetIntOpcodeParam(thread);
	int blue = CLEO_GetIntOpcodeParam(thread);
	int alpha = CLEO_GetIntOpcodeParam(thread);

	if (fixAspectRatio) {
		FixAspectRatio(&sizX, &sizY);
	}

	float magicResolutionWidth = RsGlobal.maximumWidth * 0.0015625f;
	float magicResolutionHeight = RsGlobal.maximumHeight * 0.002232143f;

	float cornerA_X = (posX - (sizX * 0.5f)) * magicResolutionWidth;
	float cornerA_Y = (posY - (sizY * 0.5f)) * magicResolutionHeight;
	float cornerB_X = ((sizX * 0.5f) + posX) * magicResolutionWidth;
	float cornerB_Y = ((sizY * 0.5f) + posY) * magicResolutionHeight;

	float *finalMaskTrisArray = maskTrisArray;
	if (maskTrisCount > 0 && maskTrisArray != nullptr) {
		finalMaskTrisArray = (float *)malloc((maskTrisCount * 2) * 4);
		int i = 0;
		int j = 1;
		while (i < (maskTrisCount * 2)) {
			finalMaskTrisArray[i] = maskTrisArray[i] * magicResolutionWidth;
			finalMaskTrisArray[j] = maskTrisArray[j] * magicResolutionHeight;
			i += 2;
			j += 2;
		}
	}

	// If no mask is used, we use a more common way to draw it, and this way we need to rotate it
	if (maskTrisCount <= 0) {
		angle += 180.0f;
		while (angle > 360.0f)
			angle -= 360.0f;
	}

	MySprite *mySprite = new MySprite(
		texture, 
		cornerA_X,
		cornerA_Y,
		cornerB_X,
		cornerB_Y,
		angle * 0.017453292f,
		depth,
		fixAspectRatio,
		maskTrisCount,
		finalMaskTrisArray,
		red, 
		green, 
		blue,
		alpha
	); 

	sprites[drawEvent].push_back(mySprite);
	return OR_CONTINUE;
}

#pragma warning( push )
#pragma warning( disable : 4244 ) // possible loss of data

// 0E62=8,print %1s% event %2d% at %3d% %4d% scale %5d% %6d% fixAR %7d% style %8d%
OpcodeResult WINAPI DRAW_STRING(CScriptThread* thread)
{
	tFontPrint *fontPrint = new tFontPrint();

	CLEO_ReadStringPointerOpcodeParam(thread, fontPrint->m_text, 128);
	int drawEvent = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_fX = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_fY = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_fWidth = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_fHeight = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_nFixAR = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nStyle = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nProp = TRUE;
	fontPrint->m_nAlign = ALIGN_LEFT;
	fontPrint->m_fWrap = 640.0f;
	fontPrint->m_nJustify = FALSE;
	fontPrint->m_Color.r = 255;
	fontPrint->m_Color.g = 255;
	fontPrint->m_Color.b = 255;
	fontPrint->m_Color.a = 255;
	fontPrint->m_nOutline = 1;
	fontPrint->m_nShadow = 0;
	fontPrint->m_DropColor.r = 0;
	fontPrint->m_DropColor.g = 0;
	fontPrint->m_DropColor.b = 0;
	fontPrint->m_DropColor.a = 255;
	fontPrint->m_nBackground = FALSE;
	fontPrint->m_BackColor.r = 0;
	fontPrint->m_BackColor.g = 0;
	fontPrint->m_BackColor.b = 0;
	fontPrint->m_BackColor.a = 0;
	textDrawer[drawEvent].m_aPrints.push_back(fontPrint);

	return OR_CONTINUE;
}

// 0E63=27,print %1s% event %2d% at %3d% %4d% scale %5d% %6d% fixAR %7d% style %8d% prop %9d% align %10d% wrap %11d% justify %12d% color %13d% %14d% %15d% %16d% outline %17d% shadow %18d%
// dropColor %19d% %20d% %21d% %22d% background %23d% backColor %24d% %25d% %26d% %27d%
OpcodeResult WINAPI DRAW_STRING_EXT(CScriptThread* thread)
{
	tFontPrint *fontPrint = new tFontPrint();

	CLEO_ReadStringPointerOpcodeParam(thread, fontPrint->m_text, 128);
	int drawEvent = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_fX = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_fY = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_fWidth = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_fHeight = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_nFixAR = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nStyle = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nProp = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nAlign = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_fWrap = CLEO_GetFloatOpcodeParam(thread);
	fontPrint->m_nJustify = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_Color.r = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_Color.g = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_Color.b = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_Color.a = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nOutline = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nShadow = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_DropColor.r = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_DropColor.g = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_DropColor.b = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_DropColor.a = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_nBackground = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_BackColor.r = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_BackColor.g = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_BackColor.b = CLEO_GetIntOpcodeParam(thread);
	fontPrint->m_BackColor.a = CLEO_GetIntOpcodeParam(thread);

	textDrawer[drawEvent].m_aPrints.push_back(fontPrint);
	return OR_CONTINUE;
}

#pragma warning( pop )  