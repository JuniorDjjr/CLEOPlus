#pragma once

class MySprite;

enum DrawEvent { BeforeDrawing, AfterDrawing, BeforeHud, AfterHud, BeforeRadar, AfterRadar, BeforeRadarOverlay, AfterRadarOverlay, BeforeBlips, AfterBlips, AfterFade, TOTAL_DRAW_EVENT };
void DrawMySprites(std::vector<MySprite*> &spritesList);
void ClearMySprites(std::vector<MySprite*> &spritesList);
extern std::vector<MySprite*> sprites[DrawEvent::TOTAL_DRAW_EVENT];
