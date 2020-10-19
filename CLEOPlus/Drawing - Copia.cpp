#include "OpcodesCommon.h"
#include "Screen.h"
#include "CHud.h"
#include "CTheScripts.h"

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
	bool fixAR;
	CRGBA rgba;
	int maskTrisCount;
	float *maskTrisArray;

	MySprite(RwTexture *newTexture, float _cornerA_X, float _cornerA_Y, float _cornerB_X, float _cornerB_Y, float _angle, int _fixAR, int _maskTrisCount, float *_maskTrisArray, int red, int green, int blue, int alpha)
	{
		texture = newTexture;
		cornerA_X = _cornerA_X;
		cornerA_Y = _cornerA_Y;
		cornerB_X = _cornerB_X;
		cornerB_Y = _cornerB_Y;
		angle = _angle;
		fixAR = _fixAR;
		maskTrisCount = _maskTrisCount;
		maskTrisArray = _maskTrisArray;
		rgba.r = red;
		rgba.g = green;
		rgba.b = blue;
		rgba.a = alpha;
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

list<MySprite*> spritesToBeDrawn;
list<MyMask*> masksToBeDrawn;

void ClearMySprites()
{
	if (spritesToBeDrawn.size() > 0) {
		for (list<MySprite*>::iterator it = spritesToBeDrawn.begin(); it != spritesToBeDrawn.end(); ++it)
		{
			MySprite *mySprite = *it;
			delete mySprite;
		}
		spritesToBeDrawn.clear();
	}
	if (masksToBeDrawn.size() > 0) {
		for (list<MyMask*>::iterator it = masksToBeDrawn.begin(); it != masksToBeDrawn.end(); ++it)
		{
			MyMask *myMask = *it;
			delete myMask;
		}
		masksToBeDrawn.clear();
	}
}
/*
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
float v19 = v15 + v14 + v9;
float v20 = v16 - v17 + v8;
float v21 = v14 - v15 + v9;
float v22 = v18 - v17 + v8;
float v23 = v15 - v14 + v9;
float v24 = v17 + v16 + v8;
float v25 = -v15 - v14 + v9;
float v26 = v18 + v17 + v8;
*/

float texCoors[16];
float verts[16];
float verts2[16];

void DrawMySprites()
{
	if (spritesToBeDrawn.size() > 0) {
		for (list<MySprite*>::iterator it = spritesToBeDrawn.begin(); it != spritesToBeDrawn.end(); ++it)
		{
			MySprite *mySprite = *it;

			RwRaster *raster = nullptr;
			if (mySprite->texture) {
				raster = mySprite->texture->raster;
			}

			CRect rect; // [esp+3Ch] [ebp-10h]

			/*rect.left = mySprite->pos.x;
			rect.top = mySprite->pos.y;
			float sizeX = mySprite->size.x * RsGlobal.maximumWidth * 0.0015625f;
			float sizeY = mySprite->size.y * RsGlobal.maximumWidth * 0.002232143f;
			CRGBA rgba = CRGBA::CRGBA(0xFF, 0xFF, 0xFF, 0xFF);*/



			mySprite->maskTrisArray[0] = 100.0f;  // x right
			mySprite->maskTrisArray[1] = 100.0f; // y top

			mySprite->maskTrisArray[2] = 200.0f;  // x left
			mySprite->maskTrisArray[3] = 100.0f; // y bottom


			mySprite->maskTrisArray[4] = 100.0f;  // x right
			mySprite->maskTrisArray[5] = 200.0f; // y top

			mySprite->maskTrisArray[6] = 200.0f;  // x left
			mySprite->maskTrisArray[7] = 250.0f; // y bottom

			mySprite->maskTrisCount = 4;


			/*verts2[0] = 100;  // x right
			verts2[1] = 100; // y top

			verts2[2] = 300;  // x left
			verts2[3] = 100; // y bottom


			verts2[4] = 100;  // x right
			verts2[5] = 300; // y top

			verts2[6] = 300;  // x left
			verts2[7] = 300; // y bottom*/


			if (mySprite->maskTrisCount > 0) {
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

				//CSprite2d::SetVertices(mySprite->maskTrisCount, mySprite->maskTrisArray, texCoors, rgba);
				CSprite2d::SetMaskVertices(mySprite->maskTrisCount, mySprite->maskTrisArray, CSprite2d::NearScreenZ);
				//CSprite2d::SetVertices(mySprite->maskTrisCount, mySprite->maskTrisArray, texCoors, rgba);
				RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
				RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRISTRIP, CSprite2d::maVertices, mySprite->maskTrisCount);
			}


			// draw texture

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



			/*v8 = (*&v1->cornerB_X + *&v1->cornerA_X) * 0.5;
			v9 = (*&v1->cornerB_Y + *&v1->cornerA_Y) * 0.5;
			v10 = v8 - *&v1->cornerA_X;
			v11 = v9 - *&v1->cornerA_Y;
			v12 = cos(*&v1->angle);
			v13 = sin(*&v1->angle);
			v14 = v12 * v11;
			v15 = v13 * v10;
			v16 = v12 * v10;
			v17 = v13 * v11;
			v18 = -v16;
			v19 = v15 + v14 + v9;
			v20 = v16 - v17 + v8;
			v21 = v14 - v15 + v9;
			v22 = v18 - v17 + v8;
			v23 = v15 - v14 + v9;
			v24 = v17 + v16 + v8;
			v25 = -v15 - v14 + v9;
			v26 = v18 + v17 + v8;
			CSprite2d::Draw((4 * v1->textureID + 0xA94B68), v26, v25, v24, v23, v22, v21, v20, v19, &v1->transparentColor);*/

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
			float v19 = v15 + v14 + v9;
			float v20 = v16 - v17 + v8;
			float v21 = v14 - v15 + v9;
			float v22 = v18 - v17 + v8;
			float v23 = v15 - v14 + v9;
			float v24 = v17 + v16 + v8;
			float v25 = -v15 - v14 + v9;
			float v26 = v18 + v17 + v8;

			//CSprite2d::SetVertices(4, verts2, texCoors, rgba);
			//CSprite2d::SetVertices(rect, rgba, rgba, rgba, rgba);
			CSprite2d::SetVertices(v26, v25, v24, v23, v22, v21, v20, v19, mySprite->rgba, mySprite->rgba, mySprite->rgba, mySprite->rgba);
			RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
			//RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
			RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
		}
		spritesToBeDrawn.clear();
	}
}


//DRAW_TEXTURE_PLUS 0 100.0 100.0 50.0 50.0 90.0 TRUE 0 0 255 255 255 255
// 0E1E=9,draw_texture_plus %1d% pos %2d% %3d% size %4d% %5d% angle %6d% fix_aspect_ratio %7d% maskTrisCount %8d% maskTrisArray %9d% rgba %10d% %11d% %12d% %13d% 
OpcodeResult WINAPI DRAW_TEXTURE_PLUS(CScriptThread* thread)
{
	RwTexture *texture = (RwTexture*)CLEO_GetIntOpcodeParam(thread);
	float posX = CLEO_GetFloatOpcodeParam(thread);
	float posY = CLEO_GetFloatOpcodeParam(thread);
	float sizX = -CLEO_GetFloatOpcodeParam(thread);
	float sizY = CLEO_GetFloatOpcodeParam(thread);
	float angle = CLEO_GetFloatOpcodeParam(thread) + 180.0f;
	int fixAspectRatio = CLEO_GetIntOpcodeParam(thread);
	int maskTrisCount = CLEO_GetIntOpcodeParam(thread);
	int maskTrisArray = CLEO_GetIntOpcodeParam(thread);
	int red = CLEO_GetIntOpcodeParam(thread);
	int green = CLEO_GetIntOpcodeParam(thread);
	int blue = CLEO_GetIntOpcodeParam(thread);
	int alpha = CLEO_GetIntOpcodeParam(thread);

	//*(&unk_A92D70 + v151) = v154 * v153;         // (pos.x - (scale.x * 0.5)) * magicResolutionWidth
	//*(&unk_A92D74 + v151) = (v147 - v152) * v156;// (pos.y - (scale.y * 0.5)) * magicResolutionHeight
	//*(&unk_A92D78 + v151) = (v150 + v146) * v153;// ((scale.x * 0.5) + pos.x) * magicResolutionWidth
	//*(&unk_A92D7C + v151) = (v152 + v147) * v156;// ((scale.y * 0.5) + pos.y) * magicResolutionHeight
	// *= 0.017453292; // rotation

	if (fixAspectRatio) {
		FixAspectRatio(&sizX, &sizY);
	}

	float magicResolutionWidth = RsGlobal.maximumWidth * 0.0015625f;
	float magicResolutionHeight = RsGlobal.maximumHeight * 0.002232143f;

	float cornerA_X = (posX - (sizX * 0.5f)) * magicResolutionWidth;
	float cornerA_Y = (posY - (sizY * 0.5f)) * magicResolutionHeight;
	float cornerB_X = ((sizX * 0.5f) + posX) * magicResolutionWidth;
	float cornerB_Y = ((sizY * 0.5f) + posY) * magicResolutionHeight;

	MySprite *mySprite = new MySprite(
		texture, 
		cornerA_X,
		cornerA_Y,
		cornerB_X,
		cornerB_Y,
		angle * 0.017453292f,
		fixAspectRatio,
		maskTrisCount,
		(float *)maskTrisArray,
		red, 
		green, 
		blue,
		alpha
	); 
	//mySprite->texture = CHud::Sprites[0].m_pTexture; //test
	spritesToBeDrawn.push_back(mySprite);
	return OR_CONTINUE;
}
