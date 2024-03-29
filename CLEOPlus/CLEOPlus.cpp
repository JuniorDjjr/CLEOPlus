#include "OpcodesCommon.h"
#include "..\injector\assembly.hpp"
#include "Patches.h"
#include "RadarBlip.h"
#include "RenderObject.h"
#include "CModelInfo.h"
#include "RenderObjectOnEntity.h"
#include "PedExtendedData.h"
#include "VehExtendedData.h"
#include "ObjExtendedData.h"
#include "CPickups.h"
#include "Drawing.h"
#include "CMenuManager.h"
#include "CStreamingInfo.h"
#include "CPedDamageResponseCalculator.h"
#include "TextDrawer/TextDrawer.h"
#include "Events.h"
#include "CStreaming.h"
#include "CTimer.h"
#include "rw/rpworld.h"
#include <set>
 
constexpr uint32_t CLEOPLUS_VERSION_INT = 0x01020000;

using namespace plugin;
using namespace std;
using namespace injector;


// -- Variables
int gameVersion = -1;
char bufferA[128] = { 0 };
char bufferB[128] = { 0 };
char bufferC[128] = { 0 };
set<int> specialCharacterModelsUsed;
bool initialized = false;
bool inSAMP = false;
bool newOpcodesInstalled = false;
bool coopOpcodesInstalled = false;
bool clipboardCommandsInstalled = false;
bool sampfuncsInstalled = false;
bool gtavhudInstalled = false;
bool blipsAlwaysRevealed = false;
float *ARwidthFromWF;
float *ARheightFromWF;
float *defaultBlipSize;
int totalZonesToReveal;
int currentSaveSlot = -2;
int timesGameRestarted = 0;
char *scriptSpace;
uintptr_t startPickups;
uintptr_t endPickups;
uint8_t sizeOfCPickup;
uint32_t sizePickups;
int* CTheScripts__ScriptConnectLodsObjects;
uint32_t sizeScriptConnectLodsObjects;
bool disableCamControl = false;
bool disabledCamControlLastFrame = false;
bool controllerMode = false;
bool pausedLastFrame = true;
bool isBuildingProcessPatched = false;
CSprite2d *radarBlipSprites;
bool keysPressedLastFrame[0xFF] = { false };
bool buttonsPressedLastFrame[2][20] = { false };
float fZero = 0.0f;
uint32_t maxDffFiles;
uintptr_t defaultMouseAccelHorizontalAddress;
uintptr_t defaultMouseAccelVerticalAddress;
uintptr_t defaultCameraMove00125;
CStreamingInfo *streamingInfoForModel;
uintptr_t addPedModelAddress;
bool disablePadControl[2] = { false };
bool disabledPadControlLastFrame[2] = { false };
bool disablePadControlMovement[2] = { false };
bool disabledPadControlMovementLastFrame[2] = { false };

const unsigned int disableCamMoveAddressesCount = 14;
uintptr_t disableCamMoveAddresses[disableCamMoveAddressesCount] = { 0x510C28, 0x50FB28, 0x50F048, 0x511E0A, 0x52228E, 0x510C1C,
	0x50F03C, 0x50FB18, 0x511DFE, 0x5222A0, 0x523F71, 0x524069, 0x525705, 0x525715 };

const unsigned int disableCamMoveAddresses00125Count = 19;
uintptr_t disableCamMoveAddresses00125[disableCamMoveAddresses00125Count] = { 0x50EF84, 0x50F032, 0x50FB10, 0x50FB7A, 0x510B5D, 0x510C12, 0x510C83, 0x511D46, 0x511DF4,
	0x5199EB, 0x522265, 0x522329, 0x523DE1, 0x523F69, 0x524061, 0x52546C, 0x5255CC, 0x5256F9, 0x5263B8 };

void asm_fld(float f) { _asm {fld  dword ptr[f]} }

typedef CVector*(__cdecl *VehFuncs_Ext_GetVehicleDummyPosAdapted)(CVehicle * vehicle, int dummyId);
VehFuncs_Ext_GetVehicleDummyPosAdapted vehFuncs_Ext_GetVehicleDummyPosAdapted;
tScriptEffectSystem *ScriptEffectSystemArray;

char* modelNames;

CdeclEvent <AddressList<0x5D19CE, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> loadingEvent;
CdeclEvent <AddressList<0x5D1907, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> loadingEventAfterPoolsLoaded;
CdeclEvent <AddressList<0x53C6DB, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> restartEvent;
CdeclEvent <AddressList<0x748E1C, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> newGameFirstStartEvent;
CdeclEvent <AddressList<0x618F51, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> startSaveGame;
ThiscallEvent <AddressList<0x5E49EF, H_CALL>, PRIORITY_AFTER, ArgPickN<CPed*, 0>, void(CPed*) >    pedAfterSetModelIndex;
ThiscallEvent <AddressList<0x6D6494, H_CALL>, PRIORITY_AFTER, ArgPickN<CVehicle*, 0>, void(CVehicle*)> vehiclePreRenderEvent;
ThiscallEvent <AddressList<0x4F77DA, H_CALL>, PRIORITY_AFTER, ArgPickN<CVehicle*, 0>, void(CVehicle*)> vehicleAfterCtor;
CdeclEvent <AddressList<0x53EB9D, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> drawAfterFade;
CdeclEvent <AddressList<0x57C2B0, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> onMenu;
CdeclEvent <AddressList<0x53BF4E, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> beforeGameProcess;
ThiscallEvent <AddressList<0x53E98B, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void(void*)> afterGameProcess;
CdeclEvent <AddressList<0x53E298, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> beforeHud;
ThiscallEvent <AddressList<0x53E52B, H_JUMP>, PRIORITY_AFTER, ArgPickNone, void(void*)> afterHud;
ThiscallEvent <AddressList<0x5E8A29, H_JUMP>, PRIORITY_BEFORE, ArgPickN<CPed*, 0>, void(CPed*)> pedPreRender;
ThiscallEvent <AddressList<0x5343B2, H_CALL>, PRIORITY_BEFORE, ArgPickN<CEntity*, 0>, void(CEntity*)> entityRenderEffects;
void CacheOnePedTask(CPed *ped, PedExtended &xdata, int &activeTaskIndex, CTask *task, bool isSecondary);
void ClearScriptLists();


// -- Opcodes

// Coop
OpcodeResult WINAPI SET_CHAR_SECOND_PLAYER(CScriptThread* thread);
OpcodeResult WINAPI DISABLE_SECOND_PLAYER(CScriptThread* thread);
OpcodeResult WINAPI FIX_TWO_PLAYERS_SEPARATED_CARS(CScriptThread* thread);

//Clipboard
OpcodeResult WINAPI READ_CLIPBOARD_DATA_TO(CScriptThread* thread);
OpcodeResult WINAPI WRITE_CLIPBOARD_DATA_FROM(CScriptThread* thread);

//NoSave
OpcodeResult WINAPI CREATE_OBJECT_NO_SAVE(CScriptThread* thread);
OpcodeResult WINAPI SET_CAR_GENERATOR_NO_SAVE(CScriptThread* thread);

//Environment
OpcodeResult WINAPI GET_CURRENT_WEATHER(CScriptThread* thread);
OpcodeResult WINAPI GET_NEXT_WEATHER(CScriptThread* thread);
OpcodeResult WINAPI SET_NEXT_WEATHER(CScriptThread* thread);
OpcodeResult WINAPI GET_RAIN_INTENSITY(CScriptThread* thread);
OpcodeResult WINAPI SET_RAIN_INTENSITY(CScriptThread* thread);
OpcodeResult WINAPI GET_CURRENT_HOUR(CScriptThread* thread);
OpcodeResult WINAPI GET_CURRENT_MINUTE(CScriptThread* thread);
OpcodeResult WINAPI GET_DAY_NIGHT_BALANCE(CScriptThread* thread);
OpcodeResult WINAPI GET_UNDERWATERNESS(CScriptThread* thread);
OpcodeResult WINAPI GET_FORCED_WEATHER(CScriptThread* thread);

//ScriptEntities
OpcodeResult WINAPI IS_CAR_SCRIPT_CONTROLLED(CScriptThread* thread);
OpcodeResult WINAPI MARK_CAR_AS_NEEDED(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_SCRIPT_CONTROLLED(CScriptThread* thread);
OpcodeResult WINAPI MARK_CHAR_AS_NEEDED(CScriptThread* thread);
OpcodeResult WINAPI IS_OBJECT_SCRIPT_CONTROLLED(CScriptThread* thread);
OpcodeResult WINAPI MARK_OBJECT_AS_NEEDED(CScriptThread* thread);

//Screen
OpcodeResult WINAPI GET_CURRENT_RESOLUTION(CScriptThread* thread);
OpcodeResult WINAPI GET_FIXED_XY_ASPECT_RATIO(CScriptThread* thread);
OpcodeResult WINAPI CONVERT_3D_TO_SCREEN_2D(CScriptThread* thread);
OpcodeResult WINAPI IS_RADAR_VISIBLE(CScriptThread* thread);
OpcodeResult WINAPI IS_HUD_VISIBLE(CScriptThread* thread);
OpcodeResult WINAPI GET_FADE_ALPHA(CScriptThread* thread);

//Drawning
OpcodeResult WINAPI DRAW_TEXTURE_PLUS(CScriptThread* thread);
OpcodeResult WINAPI GET_TEXTURE_FROM_SPRITE(CScriptThread* thread);
OpcodeResult WINAPI DRAW_STRING(CScriptThread* thread);
OpcodeResult WINAPI DRAW_STRING_EXT(CScriptThread* thread);

//Input
OpcodeResult WINAPI IS_MOUSE_WHEEL_UP(CScriptThread* thread);
OpcodeResult WINAPI IS_MOUSE_WHEEL_DOWN(CScriptThread* thread);
OpcodeResult WINAPI GET_MOUSE_SENSIBILITY(CScriptThread* thread);
OpcodeResult WINAPI IS_KEY_JUST_PRESSED(CScriptThread* thread);
OpcodeResult WINAPI IS_BUTTON_JUST_PRESSED(CScriptThread* thread);
OpcodeResult WINAPI SET_CAMERA_CONTROL(CScriptThread* thread);
OpcodeResult WINAPI IS_AIM_BUTTON_PRESSED(CScriptThread* thread);
OpcodeResult WINAPI SET_PLAYER_CONTROL_PAD(CScriptThread* thread);
OpcodeResult WINAPI SET_PLAYER_CONTROL_PAD_MOVEMENT(CScriptThread* thread);
OpcodeResult WINAPI IS_ANY_FIRE_BUTTON_PRESSED(CScriptThread* thread);
OpcodeResult WINAPI IS_SELECT_MENU_JUST_PRESSED(CScriptThread* thread);
OpcodeResult WINAPI GET_TIME_NOT_TOUCHING_PAD(CScriptThread* thread);

//Events
OpcodeResult WINAPI RETURN_SCRIPT_EVENT(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_SAVE_CONFIRMATION(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_DELETE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_CREATE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_DELETE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_CREATE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_DELETE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_CREATE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_ON_MENU(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_PROCESS(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_PROCESS(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_OBJECT_PROCESS(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_BUILDING_PROCESS(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CHAR_DAMAGE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_CAR_WEAPON_DAMAGE(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_BULLET_IMPACT(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_BEFORE_GAME_PROCESS(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_EVENT_AFTER_GAME_PROCESS(CScriptThread* thread);

//Types
OpcodeResult WINAPI GET_VEHICLE_SUBCLASS(CScriptThread* thread);
OpcodeResult WINAPI GET_ENTITY_TYPE(CScriptThread* thread);

//Extended
//Char
OpcodeResult WINAPI INIT_EXTENDED_CHAR_VARS(CScriptThread* thread);
OpcodeResult WINAPI SET_EXTENDED_CHAR_VAR(CScriptThread* thread);
OpcodeResult WINAPI GET_EXTENDED_CHAR_VAR(CScriptThread* thread);
//Car
OpcodeResult WINAPI INIT_EXTENDED_CAR_VARS(CScriptThread* thread);
OpcodeResult WINAPI SET_EXTENDED_CAR_VAR(CScriptThread* thread);
OpcodeResult WINAPI GET_EXTENDED_CAR_VAR(CScriptThread* thread);
//Obj
OpcodeResult WINAPI INIT_EXTENDED_OBJECT_VARS(CScriptThread* thread);
OpcodeResult WINAPI SET_EXTENDED_OBJECT_VAR(CScriptThread* thread);
OpcodeResult WINAPI GET_EXTENDED_OBJECT_VAR(CScriptThread* thread);

//Misc
OpcodeResult WINAPI IS_ON_MISSION(CScriptThread* thread);
OpcodeResult WINAPI IS_ON_SAMP(CScriptThread* thread);
OpcodeResult WINAPI IS_ON_CUTSCENE(CScriptThread* thread);
OpcodeResult WINAPI FIX_CHAR_GROUND_BRIGHTNESS_AND_FADE_IN(CScriptThread* thread);
OpcodeResult WINAPI IS_WEAPON_FIRE_TYPE(CScriptThread* thread);
OpcodeResult WINAPI GET_CURRENT_SAVE_SLOT(CScriptThread* thread);
OpcodeResult WINAPI IS_GAME_FIRST_START(CScriptThread* thread);
OpcodeResult WINAPI SET_CHAR_COORDINATES_SIMPLE(CScriptThread* thread);
OpcodeResult WINAPI FRAME_MOD(CScriptThread* thread);
OpcodeResult WINAPI RANDOM_PERCENT(CScriptThread* thread);
OpcodeResult WINAPI GET_TRAILER_FROM_CAR(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_FROM_TRAILER(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_DUMMY_COORD(CScriptThread* thread);
OpcodeResult WINAPI IS_CHEAT_ACTIVE(CScriptThread* thread);
OpcodeResult WINAPI CHANGE_PLAYER_MONEY(CScriptThread* thread);
OpcodeResult WINAPI CAR_HORN(CScriptThread* thread);
OpcodeResult WINAPI GET_STRING_LENGTH(CScriptThread* thread);
OpcodeResult WINAPI COPY_STRING(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_ALARM(CScriptThread* thread);
OpcodeResult WINAPI SET_CAR_ALARM(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_MAX_HEALTH(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_HEALTH_PERCENT(CScriptThread* thread);
OpcodeResult WINAPI GET_CURRENT_CAMERA_MODE(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_COLLISION_INTENSITY(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_COLLISION_COORDINATES(CScriptThread* thread);
OpcodeResult WINAPI STREAM_CUSTOM_SCRIPT_FROM_LABEL(CScriptThread* thread);
OpcodeResult WINAPI GET_LAST_CREATED_CUSTOM_SCRIPT(CScriptThread* thread);
OpcodeResult WINAPI SET_CAR_MODEL_ALPHA(CScriptThread* thread);
OpcodeResult WINAPI SET_CHAR_MODEL_ALPHA(CScriptThread* thread);
OpcodeResult WINAPI SET_OBJECT_MODEL_ALPHA(CScriptThread* thread);
OpcodeResult WINAPI GET_LOCAL_TIME(CScriptThread* thread);
OpcodeResult WINAPI SET_SCRIPT_VAR(CScriptThread* thread);
OpcodeResult WINAPI GET_SCRIPT_VAR(CScriptThread* thread);
OpcodeResult WINAPI SET_CAR_DOOR_WINDOW_STATE(CScriptThread* thread);
OpcodeResult WINAPI GET_OBJECT_CENTRE_OF_MASS_TO_BASE_OF_MODEL(CScriptThread* thread);
OpcodeResult WINAPI GET_MODEL_TYPE(CScriptThread* thread);
OpcodeResult WINAPI IS_STRING_EQUAL(CScriptThread* thread);
OpcodeResult WINAPI IS_STRING_COMMENT(CScriptThread* thread);
OpcodeResult WINAPI DOES_CAR_HAVE_PART_NODE(CScriptThread* thread);
OpcodeResult WINAPI GET_CURRENT_CHAR_WEAPONINFO(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO_MODELS(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO_FLAGS(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO_ANIMGROUP(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO_TOTAL_CLIP(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO_FIRE_TYPE(CScriptThread* thread);
OpcodeResult WINAPI GET_WEAPONINFO_SLOT(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_WEAPON_STATE(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_WEAPON_CLIP(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_COLLISION_SURFACE(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_COLLISION_LIGHTING(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_COLLISION_SURFACE(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_COLLISION_LIGHTING(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_REALLY_IN_AIR(CScriptThread* thread);
OpcodeResult WINAPI IS_CAR_REALLY_IN_AIR(CScriptThread* thread);
OpcodeResult WINAPI IS_OBJECT_REALLY_IN_AIR(CScriptThread* thread);
OpcodeResult WINAPI SIMULATE_OBJECT_DAMAGE(CScriptThread* thread);
OpcodeResult WINAPI REQUEST_PRIORITY_MODEL(CScriptThread* thread);
OpcodeResult WINAPI LOAD_ALL_PRIORITY_MODELS_NOW(CScriptThread* thread);
OpcodeResult WINAPI LOAD_SPECIAL_CHARACTER_FOR_ID(CScriptThread* thread);
OpcodeResult WINAPI UNLOAD_SPECIAL_CHARACTER_FROM_ID(CScriptThread* thread);
OpcodeResult WINAPI GET_MODEL_BY_NAME(CScriptThread* thread);
OpcodeResult WINAPI IS_MODEL_AVAILABLE_BY_NAME(CScriptThread* thread);
OpcodeResult WINAPI GET_MODEL_DOESNT_EXIST_IN_RANGE(CScriptThread* thread);
OpcodeResult WINAPI REMOVE_ALL_UNUSED_MODELS(CScriptThread* thread);
OpcodeResult WINAPI REMOVE_MODEL_IF_UNUSED(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_ON_FIRE(CScriptThread* thread);
OpcodeResult WINAPI GET_CLOSEST_COP_NEAR_CHAR(CScriptThread* thread);
OpcodeResult WINAPI GET_CLOSEST_COP_NEAR_POS(CScriptThread* thread);
OpcodeResult WINAPI GET_ANY_CHAR_NO_SAVE_RECURSIVE(CScriptThread* thread);
OpcodeResult WINAPI GET_ANY_CAR_NO_SAVE_RECURSIVE(CScriptThread* thread);
OpcodeResult WINAPI GET_ANY_OBJECT_NO_SAVE_RECURSIVE(CScriptThread* thread);
OpcodeResult WINAPI SET_CHAR_ARRESTED(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_PEDSTATE(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_PROOFS(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_PROOFS(CScriptThread* thread);
OpcodeResult WINAPI GET_OBJECT_PROOFS(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_WEAPON_VISIBLE_SET(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_STAT_ID(CScriptThread* thread);
OpcodeResult WINAPI GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS(CScriptThread* thread);
OpcodeResult WINAPI GET_OFFSET_FROM_MATRIX_IN_WORLD_COORDS(CScriptThread* thread);
OpcodeResult WINAPI SET_CAR_COORDINATES_SIMPLE(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_DAMAGE_LAST_FRAME(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_WEAPON_DAMAGE_LAST_FRAME(CScriptThread* thread);
OpcodeResult WINAPI IS_ON_SCRIPTED_CUTSCENE(CScriptThread* thread);
OpcodeResult WINAPI GET_MODEL_PED_TYPE_AND_STAT(CScriptThread* thread);
OpcodeResult WINAPI PASS_TIME(CScriptThread* thread); 
OpcodeResult WINAPI GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED(CScriptThread* thread);
OpcodeResult WINAPI GENERATE_RANDOM_FLOAT_IN_RANGE_WITH_SEED(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CAMERA_DISTANCE_TO_COORDINATES(CScriptThread* thread);
OpcodeResult WINAPI GET_FX_SYSTEM_POINTER(CScriptThread* thread);
OpcodeResult WINAPI ADD_FX_SYSTEM_PARTICLE(CScriptThread* thread);
OpcodeResult WINAPI IS_FX_SYSTEM_AVAILABLE_WITH_NAME(CScriptThread* thread);
OpcodeResult WINAPI SET_STRING_UPPER(CScriptThread* thread);
OpcodeResult WINAPI SET_STRING_LOWER(CScriptThread* thread);
OpcodeResult WINAPI STRING_FIND(CScriptThread* thread);
OpcodeResult WINAPI CUT_STRING_AT(CScriptThread* thread);
OpcodeResult WINAPI IS_STRING_CHARACTER_AT(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_RANDOM_SEED(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_RANDOM_SEED(CScriptThread* thread);
OpcodeResult WINAPI GET_OBJECT_RANDOM_SEED(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_MOVE_STATE(CScriptThread* thread);
OpcodeResult WINAPI DONT_DELETE_CHAR_UNTIL_TIME(CScriptThread* thread);
OpcodeResult WINAPI DONT_DELETE_CAR_UNTIL_TIME(CScriptThread* thread);
OpcodeResult WINAPI GET_TIME_CHAR_IS_DEAD(CScriptThread* thread);
OpcodeResult WINAPI GET_TIME_CAR_IS_DEAD(CScriptThread* thread);
OpcodeResult WINAPI SET_CHAR_IGNORE_DAMAGE_ANIMS(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_CHAR(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_CAR(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_OBJECT(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CAR_DISTANCE_TO_OBJECT(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CAR_DISTANCE_TO_CAR(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_OBJECT_DISTANCE_TO_OBJECT(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CHAR_DISTANCE_TO_COORDINATES(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_CAR_DISTANCE_TO_COORDINATES(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_OBJECT_DISTANCE_TO_COORDINATES(CScriptThread* thread);
OpcodeResult WINAPI LOCATE_ENTITY_DISTANCE_TO_ENTITY(CScriptThread* thread);
OpcodeResult WINAPI GET_ENTITY_COORDINATES(CScriptThread* thread);
OpcodeResult WINAPI GET_ENTITY_HEADING(CScriptThread* thread);
OpcodeResult WINAPI IS_CAR_OWNED_BY_PLAYER(CScriptThread* thread);
OpcodeResult WINAPI SET_CAR_OWNED_BY_PLAYER(CScriptThread* thread);
OpcodeResult WINAPI GET_MODEL_INFO(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_ANIMGROUP(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_FEAR(CScriptThread* thread);
OpcodeResult WINAPI IS_CAR_CONVERTIBLE(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_VALUE(CScriptThread* thread);
OpcodeResult WINAPI GET_CAR_PEDALS(CScriptThread* thread);
OpcodeResult WINAPI GET_LOADED_LIBRARY(CScriptThread* thread);
OpcodeResult WINAPI RETURN_TIMES(CScriptThread* thread);
OpcodeResult WINAPI GET_THIRD_PERSON_CAMERA_TARGET(CScriptThread* thread);
OpcodeResult WINAPI GET_DISTANCE_MULTIPLIER(CScriptThread* thread);
OpcodeResult WINAPI GET_ACTIVE_CAMERA_ROTATION(CScriptThread* thread);
OpcodeResult WINAPI GET_CLOSEST_WATER_DISTANCE(CScriptThread* thread);
OpcodeResult WINAPI GET_CAMERA_STRUCT(CScriptThread* thread);
OpcodeResult WINAPI GET_CAMERA_ROTATION_INPUT_VALUES(CScriptThread* thread);
OpcodeResult WINAPI SET_CAMERA_ROTATION_INPUT_VALUES(CScriptThread* thread);
OpcodeResult WINAPI SET_ON_MISSION(CScriptThread* thread);
OpcodeResult WINAPI GET_MODEL_NAME_POINTER(CScriptThread* thread);

//Math
OpcodeResult WINAPI QUAT_SLERP(CScriptThread* thread);
OpcodeResult WINAPI EASE(CScriptThread* thread);
OpcodeResult WINAPI GET_ANGLE_FROM_TWO_COORDS(CScriptThread* thread);
OpcodeResult WINAPI PERLIN_NOISE(CScriptThread* thread);
OpcodeResult WINAPI PERLIN_NOISE_FRACTAL(CScriptThread* thread);
OpcodeResult WINAPI GET_COORD_FROM_ANGLED_DISTANCE(CScriptThread* thread);
OpcodeResult WINAPI PERLIN_NOISE_FRACTAL_2D(CScriptThread* thread);
OpcodeResult WINAPI PERLIN_NOISE_FRACTAL_3D(CScriptThread* thread);
OpcodeResult WINAPI CLAMP_FLOAT(CScriptThread* thread);
OpcodeResult WINAPI CLAMP_INT(CScriptThread* thread);
OpcodeResult WINAPI SET_MATRIX_ROTATION_FROM_QUAT(CScriptThread* thread);
OpcodeResult WINAPI SET_QUAT_FROM_MATRIX(CScriptThread* thread);
OpcodeResult WINAPI ROTATE_QUAT_ON_AXIS(CScriptThread* thread);
OpcodeResult WINAPI GET_NORMALISED_QUAT(CScriptThread* thread);
OpcodeResult WINAPI MULTIPLY_QUATS(CScriptThread* thread);
OpcodeResult WINAPI INITIALISE_QUAT(CScriptThread* thread);
OpcodeResult WINAPI GET_QUAT_ELEMENTS(CScriptThread* thread);
OpcodeResult WINAPI CONVERT_DIRECTION_TO_QUAT(CScriptThread* thread);
OpcodeResult WINAPI ROTATE_MATRIX_ON_AXIS(CScriptThread* thread);
OpcodeResult WINAPI GET_MATRIX_X_ANGLE(CScriptThread* thread);
OpcodeResult WINAPI GET_MATRIX_Y_ANGLE(CScriptThread* thread);
OpcodeResult WINAPI GET_MATRIX_Z_ANGLE(CScriptThread* thread);
OpcodeResult WINAPI LERP(CScriptThread* thread);
OpcodeResult WINAPI SET_MATRIX_LOOK_DIRECTION(CScriptThread* thread);

//Bones
OpcodeResult WINAPI GET_CHAR_BONE_MATRIX(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_BONE(CScriptThread* thread);
OpcodeResult WINAPI GET_BONE_OFFSET_VECTOR(CScriptThread* thread);
OpcodeResult WINAPI GET_BONE_QUAT(CScriptThread* thread);

//Audio
OpcodeResult WINAPI GET_AUDIO_SFX_VOLUME(CScriptThread* thread);
OpcodeResult WINAPI GET_AUDIO_RADIO_VOLUME(CScriptThread* thread);
OpcodeResult WINAPI GET_AUDIOSTREAM_INTERNAL(CScriptThread* thread);

//Memory
OpcodeResult WINAPI READ_STRUCT_OFFSET(CScriptThread* thread);
OpcodeResult WINAPI WRITE_STRUCT_OFFSET(CScriptThread* thread);
OpcodeResult WINAPI COPY_MEMORY(CScriptThread* thread);
OpcodeResult WINAPI MAKE_NOP(CScriptThread* thread);
OpcodeResult WINAPI READ_STRUCT_OFFSET_MULTI(CScriptThread* thread);
OpcodeResult WINAPI WRITE_STRUCT_OFFSET_MULTI(CScriptThread* thread);
OpcodeResult WINAPI WRITE_STRUCT_PARAM(CScriptThread* thread);
OpcodeResult WINAPI READ_STRUCT_PARAM(CScriptThread* thread);

//Radar blips
OpcodeResult WINAPI ADD_CLEO_BLIP(CScriptThread* thread);
OpcodeResult WINAPI REMOVE_CLEO_BLIP(CScriptThread* thread);

//Render object
OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_CHAR_BONE(CScriptThread* thread);
OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_CHAR_BONE_FROM_SPECIAL(CScriptThread* thread);
OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_OBJECT(CScriptThread* thread);
OpcodeResult WINAPI CREATE_RENDER_OBJECT_TO_OBJECT_FROM_SPECIAL(CScriptThread* thread);
OpcodeResult WINAPI DELETE_RENDER_OBJECT(CScriptThread* thread);
OpcodeResult WINAPI SET_RENDER_OBJECT_AUTO_HIDE(CScriptThread* thread);
OpcodeResult WINAPI SET_RENDER_OBJECT_VISIBLE(CScriptThread* thread);
OpcodeResult WINAPI SET_RENDER_OBJECT_POSITION(CScriptThread* thread);
OpcodeResult WINAPI SET_RENDER_OBJECT_ROTATION(CScriptThread* thread);
OpcodeResult WINAPI SET_RENDER_OBJECT_SCALE(CScriptThread* thread);
OpcodeResult WINAPI SET_RENDER_OBJECT_DISTORTION(CScriptThread* thread);

// Pickup
OpcodeResult WINAPI GET_PICKUP_THIS_COORD(CScriptThread* thread);
OpcodeResult WINAPI GET_PICKUP_MODEL(CScriptThread* thread);
OpcodeResult WINAPI GET_PICKUP_POINTER(CScriptThread* thread);
OpcodeResult WINAPI GET_PICKUP_TYPE(CScriptThread* thread);

// Timer
OpcodeResult WINAPI DISPLAY_ONSCREEN_TIMER_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI DISPLAY_ONSCREEN_TIMER_WITH_STRING_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI DISPLAY_ONSCREEN_COUNTER_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI DISPLAY_ONSCREEN_COUNTER_WITH_STRING_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI DISPLAY_TWO_ONSCREEN_COUNTERS_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI DISPLAY_TWO_ONSCREEN_COUNTERS_WITH_STRING_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI CLEAR_ONSCREEN_TIMER_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI CLEAR_ONSCREEN_COUNTER_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI SET_ONSCREEN_COUNTER_FLASH_WHEN_FIRST_DISPLAYED_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI SET_TIMER_BEEP_COUNTDOWN_TIME_LOCAL(CScriptThread* thread);
OpcodeResult WINAPI SET_ONSCREEN_COUNTER_COLOUR_LOCAL(CScriptThread* thread);

// Intelligence
OpcodeResult WINAPI IS_CHAR_DOING_TASK_ID(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_TASK_POINTER_BY_ID(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_KILL_TARGET_CHAR(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_USING_GUN(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_FIGHTING(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_FALLEN_ON_GROUND(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_ENTERING_ANY_CAR(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_EXITING_ANY_CAR(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_PLAYING_ANY_SCRIPT_ANIMATION(CScriptThread* thread);
OpcodeResult WINAPI IS_CHAR_DOING_ANY_IMPORTANT_TASK(CScriptThread* thread);
OpcodeResult WINAPI CLEAR_CHAR_PRIMARY_TASKS(CScriptThread* thread);
OpcodeResult WINAPI CLEAR_CHAR_SECONDARY_TASKS(CScriptThread* thread);
OpcodeResult WINAPI GET_CHAR_SIMPLEST_ACTIVE_TASK(CScriptThread* thread);

// Colpoint
OpcodeResult WINAPI GET_COLLISION_BETWEEN_POINTS(CScriptThread* thread);
OpcodeResult WINAPI GET_COLPOINT_NORMAL_VECTOR(CScriptThread* thread);
OpcodeResult WINAPI GET_COLPOINT_DEPTH(CScriptThread* thread);
OpcodeResult WINAPI GET_COLPOINT_SURFACE(CScriptThread* thread);
OpcodeResult WINAPI GET_COLPOINT_LIGHTING(CScriptThread* thread);
OpcodeResult WINAPI GET_COLPOINT_COORDINATES(CScriptThread* thread);

// List
OpcodeResult WINAPI CREATE_LIST(CScriptThread* thread);
OpcodeResult WINAPI DELETE_LIST(CScriptThread* thread);
OpcodeResult WINAPI LIST_ADD(CScriptThread* thread);
OpcodeResult WINAPI LIST_REMOVE_VALUE(CScriptThread* thread);
OpcodeResult WINAPI LIST_REMOVE_INDEX(CScriptThread* thread);
OpcodeResult WINAPI GET_LIST_SIZE(CScriptThread* thread);
OpcodeResult WINAPI GET_LIST_VALUE_BY_INDEX(CScriptThread* thread);
OpcodeResult WINAPI RESET_LIST(CScriptThread* thread);
OpcodeResult WINAPI GET_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread);
OpcodeResult WINAPI LIST_ADD_STRING(CScriptThread* thread);
OpcodeResult WINAPI LIST_REMOVE_STRING_VALUE(CScriptThread* thread);
OpcodeResult WINAPI LIST_REMOVE_INDEX_RANGE(CScriptThread* thread);
OpcodeResult WINAPI REVERSE_LIST(CScriptThread* thread);
OpcodeResult WINAPI REPLACE_LIST_VALUE_BY_INDEX(CScriptThread* thread);
OpcodeResult WINAPI REPLACE_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread);
OpcodeResult WINAPI INSERT_LIST_VALUE_BY_INDEX(CScriptThread* thread);
OpcodeResult WINAPI INSERT_LIST_STRING_VALUE_BY_INDEX(CScriptThread* thread);

// Special models
OpcodeResult WINAPI LOAD_SPECIAL_MODEL(CScriptThread* thread);
OpcodeResult WINAPI REMOVE_SPECIAL_MODEL(CScriptThread* thread);
OpcodeResult WINAPI GET_SPECIAL_MODEL_DATA(CScriptThread* thread);



class CLEOPlus
{
public:
    CLEOPlus()
	{
		if (CLEO_GetVersion() < CLEO_VERSION)
		{
			MessageBox(HWND_DESKTOP, "Outdated CLEO. Update your CLEO Library.", "CLEO+.cleo", MB_ICONERROR);
			return;
		}

		gameVersion = GetGameVersion();
		if (gameVersion != GAME_10US_HOODLUM && gameVersion != GAME_10US_COMPACT) {
			MessageBox(HWND_DESKTOP, "CLEO+ is only compatible with EXE 1.0 US (Hoodlum or Compact)", "CLEO+.cleo", MB_ICONERROR);
			return;
		}

		Events::initRwEvent.after +=[]
		{
			if (initialized) return;

			if (GetModuleHandleA("SAMP.dll")) {
				inSAMP = true;
			}
			else inSAMP = false;

			if (GetModuleHandleA("newOpcodes.cleo")) {
				newOpcodesInstalled = true;
			}
			else newOpcodesInstalled = false;

			if (GetModuleHandleA("CoopOpcodes.cleo")) {
				coopOpcodesInstalled = true;
			}
			else coopOpcodesInstalled = false;

			if (GetModuleHandleA("ClipboardCommands.cleo")) {
				clipboardCommandsInstalled = true;
			}
			else clipboardCommandsInstalled = false;

			if (GetModuleHandleA("SAMPFUNCS.asi")) {
				sampfuncsInstalled = true;
			}
			else sampfuncsInstalled = false;

			if (GetModuleHandleA("V_HUD_by_DK22Pac.asi") || GetModuleHandleA("VHud.asi")) {
				gtavhudInstalled = true;
			}
			else gtavhudInstalled = false;

			HINSTANCE moduleVehFuncs = GetModuleHandleA("VehFuncs.asi");
			if (moduleVehFuncs) {
				vehFuncs_Ext_GetVehicleDummyPosAdapted = (VehFuncs_Ext_GetVehicleDummyPosAdapted)GetProcAddress(moduleVehFuncs, "Ext_GetVehicleDummyPosAdapted");
			}
			else {
				moduleVehFuncs = 0;
				vehFuncs_Ext_GetVehicleDummyPosAdapted = 0;
			}


			//------------------------------------------------------------------------------------


			//Coop
			if (!coopOpcodesInstalled) {
				CLEO_RegisterOpcode(0xEA0, SET_CHAR_SECOND_PLAYER); // 0xEA0=3,set_char_second_player %1d% enable_camera %2d% separate_cars %3d%
				CLEO_RegisterOpcode(0xEA1, DISABLE_SECOND_PLAYER); // 0xEA1=1,disable_second_player_restore_camera %1d%
				CLEO_RegisterOpcode(0xEA2, FIX_TWO_PLAYERS_SEPARATED_CARS); // 0xEA2=1,fix_two_players_separated_cars %1d%
			}

			//Clipboard
			if (!clipboardCommandsInstalled && !sampfuncsInstalled) {
				CLEO_RegisterOpcode(0xB20, READ_CLIPBOARD_DATA_TO); // 0xB20=2,read_clipboard_data_to %1d% size %2d%
				CLEO_RegisterOpcode(0xB21, WRITE_CLIPBOARD_DATA_FROM); // 0xB21=2,write_clipboard_data_from %1d% size %2d%
			}

			//NoSave
			CLEO_RegisterOpcode(0xE01, CREATE_OBJECT_NO_SAVE); // 0xE01=7,create_object_no_save %1o% at %2d% %3d% %4d% offset %5d% ground %6d% to %7d%
			CLEO_RegisterOpcode(0xE02, SET_CAR_GENERATOR_NO_SAVE); // 0xE02=1,set_car_generator_no_save %1d%

			//Environment
			if (!newOpcodesInstalled) CLEO_RegisterOpcode(0xD59, GET_CURRENT_WEATHER); // 0xD59=1,get_current_weather_to %1d%
			CLEO_RegisterOpcode(0xE04, GET_NEXT_WEATHER); // 0xE04=1,get_next_weather_to %1d%
			CLEO_RegisterOpcode(0xE05, SET_NEXT_WEATHER); // 0xE05=1,set_next_weather_to %1d%
			CLEO_RegisterOpcode(0xE06, GET_RAIN_INTENSITY); // 0xE06=1,get_rain_intensity %1d%
			CLEO_RegisterOpcode(0xE07, SET_RAIN_INTENSITY); // 0xE07=1,set_rain_intensity %1d%
			CLEO_RegisterOpcode(0xE40, GET_CURRENT_HOUR); // 0xE40=1,get_current_hour %1d%
			CLEO_RegisterOpcode(0xE41, GET_CURRENT_MINUTE); // 0xE41=1,get_current_minute %1d%
			CLEO_RegisterOpcode(0xE6C, GET_DAY_NIGHT_BALANCE); // 0xE6C=1,get_day_night_balance %1d%
			CLEO_RegisterOpcode(0xE6D, GET_UNDERWATERNESS); // 0xE6D=1,get_underwaterness %1d%
			CLEO_RegisterOpcode(0xEB0, GET_FORCED_WEATHER); // 0xEB0=1,get_forced_weather %1d%

			//ScriptEntities
			CLEO_RegisterOpcode(0xE08, IS_CAR_SCRIPT_CONTROLLED); // 0xE08=1,is_car_script_controlled %1d%
			CLEO_RegisterOpcode(0xE09, MARK_CAR_AS_NEEDED); // 0xE09=1,mark_car_as_needed %1d%
			CLEO_RegisterOpcode(0xE0A, IS_CHAR_SCRIPT_CONTROLLED); // 0xE0A=1,is_char_script_controlled %1d%
			CLEO_RegisterOpcode(0xE0B, MARK_CHAR_AS_NEEDED); // 0xE0B=1,mark_char_as_needed %1d%
			CLEO_RegisterOpcode(0xE0C, IS_OBJECT_SCRIPT_CONTROLLED); // 0xE0C=1,is_object_script_controlled %1d%
			CLEO_RegisterOpcode(0xE0D, MARK_OBJECT_AS_NEEDED); // 0xE0D=1,mark_object_as_needed %1d%

			//Screen
			CLEO_RegisterOpcode(0xE0E, GET_CURRENT_RESOLUTION); // 0xE0E=2,get_current_resolution_to %1d% %2d%
			CLEO_RegisterOpcode(0xE0F, GET_FIXED_XY_ASPECT_RATIO); // 0xE0F=4,get_fixed_xy_aspect_ratio %1d% %2d% to %3d% %4d%
			CLEO_RegisterOpcode(0xE3F, CONVERT_3D_TO_SCREEN_2D); // 0xE3F=9,convert_3d_to_screen_2d %1d% %2d% %3d% checkNearClip %4d% checkFarClip %5d% store_2d_to %6d% %7d% size_to %8d% %9d%
			CLEO_RegisterOpcode(0xEB8, IS_RADAR_VISIBLE); // 0xEB8=0,is_radar_visible
			CLEO_RegisterOpcode(0xEB9, IS_HUD_VISIBLE); // 0xEB9=0,is_hud_visible
			CLEO_RegisterOpcode(0xEC7, GET_FADE_ALPHA); // 0xEC7=1,get_fade_alpha %1d%

			//Input
			CLEO_RegisterOpcode(0xE10, IS_MOUSE_WHEEL_UP); // 0xE10=0,is_mouse_wheel_up
			CLEO_RegisterOpcode(0xE11, IS_MOUSE_WHEEL_DOWN); // 0xE11=0,is_mouse_wheel_down
			CLEO_RegisterOpcode(0xE23, GET_MOUSE_SENSIBILITY); // 0xE23=1,get_mouse_sensibility_to %1d%
			CLEO_RegisterOpcode(0xE3D, IS_KEY_JUST_PRESSED); // 0xE3D=1,is_key_just_pressed %1d%
			CLEO_RegisterOpcode(0xE3E, IS_BUTTON_JUST_PRESSED); // 0xE3E=2,is_button_just_pressed %1d% button %2d%
			CLEO_RegisterOpcode(0xE60, SET_CAMERA_CONTROL); // 0xE60=1,set_camera_control %1d%
			CLEO_RegisterOpcode(0xE67, IS_AIM_BUTTON_PRESSED); // 0xE67=1,is_aim_button_pressed %1d%
			CLEO_RegisterOpcode(0xE68, SET_PLAYER_CONTROL_PAD); // 0xE68=2,set_player_control_pad %1d% %2d% 
			CLEO_RegisterOpcode(0xE69, SET_PLAYER_CONTROL_PAD_MOVEMENT); // 0xE69=2,set_player_control_pad_movement %1d% %2d% 
			CLEO_RegisterOpcode(0xE8D, IS_ANY_FIRE_BUTTON_PRESSED); // 0xE8D=1,is_any_fire_button_pressed %1d%
			CLEO_RegisterOpcode(0xE6E, IS_SELECT_MENU_JUST_PRESSED); // 0xE6E=0,is_select_menu_just_pressed
			CLEO_RegisterOpcode(0xF13, GET_TIME_NOT_TOUCHING_PAD); // 0xF13=2,get_time_not_touching_pad %1d% store_to %2d%

			//Events
			CLEO_RegisterOpcode(0xED0, RETURN_SCRIPT_EVENT); // 0xED0=0,return_script_event
			CLEO_RegisterOpcode(0xED1, SET_SCRIPT_EVENT_SAVE_CONFIRMATION); // 0xED1=3,set_script_event_save_confirmation %1d% label %2p% var_slot %3d%
			CLEO_RegisterOpcode(0xED2, SET_SCRIPT_EVENT_CHAR_DELETE); // 0xED2=3,set_script_event_char_delete %1d% label %2p% var_char %3d%
			CLEO_RegisterOpcode(0xED3, SET_SCRIPT_EVENT_CHAR_CREATE); // 0xED3=3,set_script_event_char_create %1d% label %2p% var_char %3d%
			CLEO_RegisterOpcode(0xED4, SET_SCRIPT_EVENT_CAR_DELETE); // 0xED4=3,set_script_event_car_delete %1d% label %2p% var_car %3d%
			CLEO_RegisterOpcode(0xED5, SET_SCRIPT_EVENT_CAR_CREATE); // 0xED5=3,set_script_event_car_create %1d% label %2p% var_car %3d%
			CLEO_RegisterOpcode(0xED6, SET_SCRIPT_EVENT_OBJECT_DELETE); // 0xED6=3,set_script_event_object_delete %1d% label %2p% var_object %3d%
			CLEO_RegisterOpcode(0xED7, SET_SCRIPT_EVENT_OBJECT_CREATE); // 0xED7=3,set_script_event_object_create %1d% label %2p% var_object %3d%
			CLEO_RegisterOpcode(0xED8, SET_SCRIPT_EVENT_ON_MENU); // 0xED8=3,set_script_event_on_menu %1d% label %2p% var_just_paused %3d%
			CLEO_RegisterOpcode(0xEDA, SET_SCRIPT_EVENT_CHAR_PROCESS); // 0xEDA=3,set_script_event_char_process %1d% label %2p% var_char %3d%
			CLEO_RegisterOpcode(0xEDB, SET_SCRIPT_EVENT_CAR_PROCESS); // 0xEDB=3,set_script_event_car_process %1d% label %2p% var_car %3d%
			CLEO_RegisterOpcode(0xEDC, SET_SCRIPT_EVENT_OBJECT_PROCESS); // 0xEDC=3,set_script_event_object_process %1d% label %2p% var_object %3d%
			CLEO_RegisterOpcode(0xEDD, SET_SCRIPT_EVENT_BUILDING_PROCESS); // 0xEDD=3,set_script_event_building_process %1d% label %2p% var_building %3d%
			CLEO_RegisterOpcode(0xEDE, SET_SCRIPT_EVENT_CHAR_DAMAGE); // 0xEDE=3,set_script_event_char_damage %1d% label %2p% var_char %3d%
			CLEO_RegisterOpcode(0xEDF, SET_SCRIPT_EVENT_CAR_WEAPON_DAMAGE); // 0xEDF=3,set_script_event_car_weapon_damage %1d% label %2p% var_car %3d%
			CLEO_RegisterOpcode(0xEE0, SET_SCRIPT_EVENT_BULLET_IMPACT); // 0xEE0=6,set_script_event_bullet_impact %1d% label %2p% var_owner %3d% var_victim %4d% var_weapon %5d% var_colpoint %6d%
			CLEO_RegisterOpcode(0xF0B, SET_SCRIPT_EVENT_BEFORE_GAME_PROCESS); // 0xF0B=2,set_script_event_before_game_process %1d% label %2p%
			CLEO_RegisterOpcode(0xF0C, SET_SCRIPT_EVENT_AFTER_GAME_PROCESS); // 0xF0C=2,set_script_event_after_game_process %1d% label %2p%

			//Types
			CLEO_RegisterOpcode(0xE12, GET_VEHICLE_SUBCLASS); // 0xE12=2,get_vehicle %1d% subclass_to %2d%
			CLEO_RegisterOpcode(0xE13, GET_ENTITY_TYPE); // 0xE13=2,get_entity %1d% type_to %2d%

			//Extended
			//CHAR
			CLEO_RegisterOpcode(0xE14, INIT_EXTENDED_CHAR_VARS); // 0xE14=3,init_extended_char_vars %1d% id %2d% new_vars %3d%
			CLEO_RegisterOpcode(0xE15, SET_EXTENDED_CHAR_VAR); // 0xE15=4,set_extended_char_var %1d% id %2d% var %3d% value %4d%
			CLEO_RegisterOpcode(0xE16, GET_EXTENDED_CHAR_VAR); // 0xE16=4,get_extended_char_var %1d% id %2d% var %3d% to %4d%

			//CAR
			CLEO_RegisterOpcode(0xE17, INIT_EXTENDED_CAR_VARS); // 0xE17=3,init_extended_car_vars %1d% id %2d% new_vars %3d%
			CLEO_RegisterOpcode(0xE18, SET_EXTENDED_CAR_VAR); // 0xE18=4,set_extended_car_var %1d% id %2d% var %3d% value %4d%
			CLEO_RegisterOpcode(0xE19, GET_EXTENDED_CAR_VAR); // 0xE19=4,get_extended_car_var %1d% id %2d% var %3d% to %4d%

			//OBJECT
			CLEO_RegisterOpcode(0xE1A, INIT_EXTENDED_OBJECT_VARS); // 0xE1A=3,init_extended_object_vars %1d% id %2d% new_vars %3d%
			CLEO_RegisterOpcode(0xE1B, SET_EXTENDED_OBJECT_VAR); // 0xE1B=4,set_extended_object_var %1d% id %2d% var %3d% value %4d%
			CLEO_RegisterOpcode(0xE1C, GET_EXTENDED_OBJECT_VAR); // 0xE1C=4,get_extended_object_var %1d% id %2d% var %3d% to %4d%

			// Misc
			CLEO_RegisterOpcode(0xE1D, IS_ON_MISSION); // 0xE1D=0,is_on_mission
			CLEO_RegisterOpcode(0xE20, IS_ON_SAMP); // 0xE20=0,is_on_samp
			CLEO_RegisterOpcode(0xE25, IS_ON_CUTSCENE); // 0xE25=0,is_on_cutscene
			CLEO_RegisterOpcode(0xE24, FIX_CHAR_GROUND_BRIGHTNESS_AND_FADE_IN); // 0xE24=4,fix_char %1d% ground %2d% brightness %3d% and_fade_in %4d%
			CLEO_RegisterOpcode(0xE26, IS_WEAPON_FIRE_TYPE); // 0xE26=2,is_weapon %1d% fire_type %2d%
			CLEO_RegisterOpcode(0xE2C, GET_CURRENT_SAVE_SLOT); // 0xE2C=1,get_current_save_slot %1d%
			CLEO_RegisterOpcode(0xE2D, IS_GAME_FIRST_START); // 0xE2D=0,is_game_first_start
			CLEO_RegisterOpcode(0xE32, SET_CHAR_COORDINATES_SIMPLE); // 0xE32=4,set_char_coordinates_simple %1d% coord %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xE45, FRAME_MOD); // 0xE45=1,frame_mod %1d%
			CLEO_RegisterOpcode(0xE4D, RANDOM_PERCENT); // 0xE4D=1,random_percent %1d%
			CLEO_RegisterOpcode(0xE59, GET_TRAILER_FROM_CAR); // 0xE59=2,get_trailer_from_car %1d% trailer %2d%
			CLEO_RegisterOpcode(0xE5A, GET_CAR_FROM_TRAILER); // 0xE5A=2,get_car_from_trailer %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE5B, GET_CAR_DUMMY_COORD); // 0xE5B=7,get_car_dummy_coord %1d% dummy %2d% world_coords %3d% invert_x %4d% store_to %5d% %6d% %7d% // same as NewOpcodes but this is adapted to VehFuncs
			CLEO_RegisterOpcode(0xE5D, IS_CHEAT_ACTIVE); // 0xE5D=1,is_cheat_active %1d%
			CLEO_RegisterOpcode(0xE5E, CHANGE_PLAYER_MONEY); // 0xE5E=3,change_player_money %1d% mode %2d% value %3d%
			CLEO_RegisterOpcode(0xE5F, CAR_HORN); // 0xE5F=1,car_horn %1d%
			if (!newOpcodesInstalled) CLEO_RegisterOpcode(0xD4C, GET_STRING_LENGTH); // 0xD4C=2,%2d% = string %1s% length // keep NewOpcodes order
			if (!newOpcodesInstalled) CLEO_RegisterOpcode(0xD4D, COPY_STRING); // 0xD4D=2,copy_string %1s% to %2d%
			CLEO_RegisterOpcode(0xE00, GET_CAR_ALARM); // 0xE00=2,get_car_alarm %1d% mode_to %2d%
			CLEO_RegisterOpcode(0xE61, SET_CAR_ALARM); // 0xE61=2,set_car_alarm %1d% mode %2d%
			if (!newOpcodesInstalled) CLEO_RegisterOpcode(0xD39, GET_CHAR_MAX_HEALTH); // 0xD39=2,%2d% = get_char_max_health %1d% // keep NewOpcodes order
			CLEO_RegisterOpcode(0xE5C, GET_CHAR_HEALTH_PERCENT); // 0xE5C=2,get_player_health_percent %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE64, GET_CURRENT_CAMERA_MODE); // 0xE64=1,get_camera_mode %1d%
			CLEO_RegisterOpcode(0xE65, GET_CAR_COLLISION_INTENSITY); // 0xE65=2,get_car_collision_intensity %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE66, GET_CAR_COLLISION_COORDINATES); // 0xE66=4,get_car_collision_coordinates %1d% store_to %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xE6F, STREAM_CUSTOM_SCRIPT_FROM_LABEL); // 0xE6F=-1,stream_custom_script_from_label %1p%
			CLEO_RegisterOpcode(0xE70, GET_LAST_CREATED_CUSTOM_SCRIPT); // 0xE70=1,get_last_created_custom_script %1d%
			CLEO_RegisterOpcode(0xE71, GET_OBJECT_CENTRE_OF_MASS_TO_BASE_OF_MODEL); // 0xE71=2,get_object_centre_of_mass_to_base_of_model %1d% %2d%
			CLEO_RegisterOpcode(0xE7F, GET_MODEL_TYPE); // 0xE7F=2,get_model_type %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE80, IS_STRING_EQUAL); // 0xE80=5,is_string_equal %1s% %2s% max_size %3d% case_sensitive %4d% ignore_charactere %5s%
			CLEO_RegisterOpcode(0xE81, IS_STRING_COMMENT); // 0xE81=1,is_string_comment %1s%
			CLEO_RegisterOpcode(0xE82, DOES_CAR_HAVE_PART_NODE); // 0xE82=2,does_car_have_part_node %1d% %2d%
			CLEO_RegisterOpcode(0xE83, GET_CURRENT_CHAR_WEAPONINFO); // 0xE83=2,get_current_char_weaponinfo %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE84, GET_WEAPONINFO); // 0xE84=3,get_weaponinfo %1d% skill %2d% store_to %3d%
			CLEO_RegisterOpcode(0xE85, GET_WEAPONINFO_MODELS); // 0xE85=3,get_weaponinfo_models %1d% store_to %2d% %3d%
			CLEO_RegisterOpcode(0xE86, GET_WEAPONINFO_FLAGS); // 0xE86=2,get_weaponinfo_flags %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE87, GET_WEAPONINFO_ANIMGROUP); // 0xE87=2,get_weaponinfo_animgroup %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE88, GET_WEAPONINFO_TOTAL_CLIP); // 0xE88=2,get_weaponinfo_total_clip %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE89, GET_WEAPONINFO_FIRE_TYPE); // 0xE89=2,get_weaponinfo_fire_type %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE8A, GET_WEAPONINFO_SLOT); // 0xE8A=2,get_weaponinfo_slot %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE8B, GET_CHAR_WEAPON_STATE); // 0xE8B=2,get_char_weapon_state %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE8C, GET_CHAR_WEAPON_CLIP); // 0xE8C=2,get_char_weapon_clip %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE8E, GET_CHAR_COLLISION_SURFACE); // 0xE8E=2,get_char_collision_surface %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE8F, GET_CHAR_COLLISION_LIGHTING); // 0xE8F=2,get_char_collision_lighting %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE90, GET_CAR_COLLISION_SURFACE); // 0xE90=2,get_car_collision_surface %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE91, GET_CAR_COLLISION_LIGHTING); // 0xE91=2,get_car_collision_lighting %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE92, IS_CHAR_REALLY_IN_AIR); // 0xE92=1,is_char_really_in_air %1d%
			CLEO_RegisterOpcode(0xE93, IS_CAR_REALLY_IN_AIR); // 0xE93=1,is_car_really_in_air %1d%
			CLEO_RegisterOpcode(0xE94, IS_OBJECT_REALLY_IN_AIR); // 0xE94=1,is_object_really_in_air %1d%
			CLEO_RegisterOpcode(0xE95, SIMULATE_OBJECT_DAMAGE); // 0xE95=3,simulate_object_damage %1d% damage %2d% type %3d%
			CLEO_RegisterOpcode(0xE98, REQUEST_PRIORITY_MODEL); // 0xE98=1,request_priority_model %1d%
			CLEO_RegisterOpcode(0xE99, LOAD_ALL_PRIORITY_MODELS_NOW); // 0xE99=0,load_all_priority_models_now
			CLEO_RegisterOpcode(0xE9A, LOAD_SPECIAL_CHARACTER_FOR_ID); // 0xE9A=2,load_special_character_for_id %1d% name %2d%
			CLEO_RegisterOpcode(0xE9B, UNLOAD_SPECIAL_CHARACTER_FROM_ID); // 0xE9B=1,unload_special_character_from_id %1d%
			CLEO_RegisterOpcode(0xE9C, GET_MODEL_BY_NAME); // 0xE9C=2,get_model_by_name %1d% store_id %2d%
			CLEO_RegisterOpcode(0xE9D, IS_MODEL_AVAILABLE_BY_NAME); // 0xE9D=1,is_model_available_by_name %1d%
			CLEO_RegisterOpcode(0xE9E, GET_MODEL_DOESNT_EXIST_IN_RANGE); // 0xE9E=3,get_model_doesnt_exist_in_range %1d% to %2d% store_to %3d%
			CLEO_RegisterOpcode(0xE9F, REMOVE_ALL_UNUSED_MODELS); // 0xE9F=0,remove_all_unused_models
			CLEO_RegisterOpcode(0xEA3, REMOVE_MODEL_IF_UNUSED); // 0xEA3=1,remove_model_if_unused %1d%
			CLEO_RegisterOpcode(0xEA4, IS_CHAR_ON_FIRE); // 0xEA4=1,is_char_on_fire %1d%
			CLEO_RegisterOpcode(0xEA5, GET_CLOSEST_COP_NEAR_CHAR); // 0xEA5=7,get_closest_cop_near_char %1d% radius %2d% alive %3d% in_car %4d% on_foot %5d% seen_in_front %6d% store_to %7d%
			CLEO_RegisterOpcode(0xEA6, GET_CLOSEST_COP_NEAR_POS); // 0xEA6=8,get_closest_cop_near_char %1d% %2d% %3d% radius %4d% alive %5d% in_car %6d% on_foot %7d% store_to %8d%
			CLEO_RegisterOpcode(0xEA7, GET_ANY_CHAR_NO_SAVE_RECURSIVE); // 0xEA7=3,get_any_char_no_save_recursive %1d% progress_to %2d% char_to %3d%
			CLEO_RegisterOpcode(0xEA8, GET_ANY_CAR_NO_SAVE_RECURSIVE); // 0xEA8=3,get_any_car_no_save_recursive %1d% progress_to %2d% car_to %3d%
			CLEO_RegisterOpcode(0xEA9, GET_ANY_OBJECT_NO_SAVE_RECURSIVE); // 0xEA9=3,get_any_object_no_save_recursive %1d% progress_to %2d% object_to %3d%
			CLEO_RegisterOpcode(0xEAA, SET_CHAR_ARRESTED); // 0xEAA=1,set_char_arrested %1d%
			CLEO_RegisterOpcode(0xEAB, GET_CHAR_PEDSTATE); // 0xEAB=2,get_char_pedstate %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEAC, GET_CHAR_PROOFS); // 0xEAC=6,get_char_proofs %1d% bullet %2d% fire %3d% explosion %4d% collision %5d% melee %6d%
			CLEO_RegisterOpcode(0xEAD, GET_CAR_PROOFS); // 0xEAD=6,get_car_proofs %1d% bullet %2d% fire %3d% explosion %4d% collision %5d% melee %6d%
			CLEO_RegisterOpcode(0xEAE, GET_OBJECT_PROOFS); // 0xEAE=6,get_object_proofs %1d% bullet %2d% fire %3d% explosion %4d% collision %5d% melee %6d%
			CLEO_RegisterOpcode(0xEAF, IS_CHAR_WEAPON_VISIBLE_SET); // 0xEAF=1,is_char_weapon_visible_set %1d%
			CLEO_RegisterOpcode(0xEB1, GET_CHAR_STAT_ID); // 0xEB1=2,get_char_stat_id %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEB2, GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS); // 0xEB2=6,get_offset_from_camera_in_world_coords %1d% %2d% %3d% store_to %4d% %5d% %6d%
			if (!newOpcodesInstalled) CLEO_RegisterOpcode(0xD0A, GET_OFFSET_FROM_MATRIX_IN_WORLD_COORDS); // 0xD0A=7,store_coords_to %5d% %6d% %7d% from_matrix %1d% with_offsets %2d% %3d% %4d% // keep NewOpcodes order
			CLEO_RegisterOpcode(0xEB4, SET_CAR_COORDINATES_SIMPLE); // 0xEB4=4,set_car_coordinates_simple %1d% position %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xEB5, GET_CHAR_DAMAGE_LAST_FRAME); // 0xEB5=5,get_char_damage_last_frame %1d% damager %2d% type %3d% part %4d% intensity %5d%
			CLEO_RegisterOpcode(0xEB6, GET_CAR_WEAPON_DAMAGE_LAST_FRAME); // 0xEB6=4,get_car_weapon_damage_last_frame %1d% char %2d% type %3d% intensity %4d%
			CLEO_RegisterOpcode(0xEB7, IS_ON_SCRIPTED_CUTSCENE); // 0xEB7=0,is_on_scripted_cutscene
			CLEO_RegisterOpcode(0xEBA, GET_MODEL_PED_TYPE_AND_STAT); // 0xEBA=3,get_model_ped_type_and_stat %1d% store_to %2d% %3d%
			CLEO_RegisterOpcode(0xEBB, PASS_TIME); // 0xEBB=1,pass_time %1d%
			CLEO_RegisterOpcode(0xEBC, GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED); // 0xEBC=4,generate_random_int_in_range_with_seed %1d% min %2d% max %3d% store_to %4d%
			CLEO_RegisterOpcode(0xEBD, GENERATE_RANDOM_FLOAT_IN_RANGE_WITH_SEED); // 0xEBD=4,generate_random_float_in_range_with_seed %1d% min %2d% max %3d% store_to %4d%
			CLEO_RegisterOpcode(0xEBE, LOCATE_CAMERA_DISTANCE_TO_COORDINATES); // 0xEBE=4,locate_camera_distance_to_coordinates %1d% %2d% %3d% radius %4d%
			CLEO_RegisterOpcode(0xEBF, GET_FX_SYSTEM_POINTER); // 0xEBF=2,get_fx_system_pointer %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEC0, ADD_FX_SYSTEM_PARTICLE); // 0xEC0=14,add_fx_system_particle %1d% coord %2d% %3d% %4d% vel %5d% %6d% %7d% size %8d% brightness %9d% rgba %10d% %11d% %12d% %13d% lastFactor %14d%
			CLEO_RegisterOpcode(0xEC1, IS_FX_SYSTEM_AVAILABLE_WITH_NAME); // 0xEC1=1,is_fx_system_available_with_name %1s%
			CLEO_RegisterOpcode(0xEC2, SET_STRING_UPPER); // 0xEC2=1,set_string_upper %1s%
			CLEO_RegisterOpcode(0xEC3, SET_STRING_LOWER); // 0xEC3=1,set_string_lower %1s%
			CLEO_RegisterOpcode(0xEC4, STRING_FIND); // 0xEC4=4,string_find %1d% %2s% %3s% store_to %4d%
			CLEO_RegisterOpcode(0xEC5, CUT_STRING_AT); // 0xEC5=2,cut_string_at %1d% %2d%
			CLEO_RegisterOpcode(0xEC6, IS_STRING_CHARACTER_AT); // 0xEC6=3,is_string_character_at %1s% character %2s% index %3d%
			CLEO_RegisterOpcode(0xEC8, GET_CHAR_RANDOM_SEED); // 0xEC8=2,get_char_random_seed %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEC9, GET_CAR_RANDOM_SEED); // 0xEC9=2,get_car_random_seed %1d% store_to %2d%
			CLEO_RegisterOpcode(0xECA, GET_OBJECT_RANDOM_SEED); // 0xECA=2,get_object_random_seed %1d% store_to %2d%
			CLEO_RegisterOpcode(0xECB, GET_CHAR_MOVE_STATE); // 0xECB=2,get_char_move_state %1d% store_to %2d%
			CLEO_RegisterOpcode(0xECC, DONT_DELETE_CHAR_UNTIL_TIME); // 0xECC=2,dont_delete_char_until_time %1d% %2d%
			CLEO_RegisterOpcode(0xECD, DONT_DELETE_CAR_UNTIL_TIME); // 0xECD=2,dont_delete_car_until_time %1d% %2d%
			CLEO_RegisterOpcode(0xECE, GET_TIME_CHAR_IS_DEAD); // 0xECE=2,get_time_char_is_dead %1d% store_to %2d%
			CLEO_RegisterOpcode(0xECF, GET_TIME_CAR_IS_DEAD); // 0xECF=2,get_time_car_is_dead %1d% store_to %2d%
			CLEO_RegisterOpcode(0xED9, SET_CHAR_IGNORE_DAMAGE_ANIMS); // 0xED9=2,set_char_ignore_damage_anims %1d% %2d%
			CLEO_RegisterOpcode(0xEE4, LOCATE_CHAR_DISTANCE_TO_CHAR); // 0xEE4=3,locate_char_distance_to_char %1d% char %2d% radius %3d%
			CLEO_RegisterOpcode(0xEE5, LOCATE_CHAR_DISTANCE_TO_CAR); // 0xEE5=3,locate_char_distance_to_car %1d% car %2d% radius %3d%
			CLEO_RegisterOpcode(0xEE6, LOCATE_CHAR_DISTANCE_TO_OBJECT); // 0xEE6=3,locate_char_distance_to_object %1d% object %2d% radius %3d%
			CLEO_RegisterOpcode(0xEE7, LOCATE_CAR_DISTANCE_TO_OBJECT); // 0xEE7=3,locate_car_distance_to_object %1d% object %2d% radius %3d%
			CLEO_RegisterOpcode(0xEE8, LOCATE_CAR_DISTANCE_TO_CAR); // 0xEE8=3,locate_car_distance_to_car %1d% car %2d% radius %3d%
			CLEO_RegisterOpcode(0xEE9, LOCATE_OBJECT_DISTANCE_TO_OBJECT); // 0xEE9=3,locate_object_distance_to_object %1d% object %2d% radius %3d%
			CLEO_RegisterOpcode(0xEEA, LOCATE_CHAR_DISTANCE_TO_COORDINATES); // 0xEEA=5,locate_char_distance_to_coordinates %1d% pos %2d% %3d% %4d% radius %5d%
			CLEO_RegisterOpcode(0xEEB, LOCATE_CAR_DISTANCE_TO_COORDINATES); // 0xEEB=5,locate_car_distance_to_coordinates %1d% pos %2d% %3d% %4d% radius %5d%
			CLEO_RegisterOpcode(0xEEC, LOCATE_OBJECT_DISTANCE_TO_COORDINATES); // 0xEEC=5,locate_object_distance_to_coordinates %1d% pos %2d% %3d% %4d% radius %5d%
			CLEO_RegisterOpcode(0xEED, LOCATE_ENTITY_DISTANCE_TO_ENTITY); // 0xEED=5,locate_entity_distance_to_entity %1d% %2d% radius %5d%
			CLEO_RegisterOpcode(0xEEE, GET_ENTITY_COORDINATES); // 0xEEE=4,get_entity_coordinates %1d% store_to %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xEEF, GET_ENTITY_HEADING); // 0xEEF=2,get_entity_heading %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEF5, IS_CAR_OWNED_BY_PLAYER); // 0xEF5=1,is_car_owned_by_player %1d%
			CLEO_RegisterOpcode(0xEF6, SET_CAR_OWNED_BY_PLAYER); // 0xEF6=2,set_car_owned_by_player %1d% %2d%
			CLEO_RegisterOpcode(0xEF8, GET_MODEL_INFO); // 0xEF8=2,get_model_info %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEF9, GET_CAR_ANIMGROUP); // 0xEF9=2,get_car_animgroup %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEFA, GET_CHAR_FEAR); // 0xEFA=2,get_char_fear %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEFB, IS_CAR_CONVERTIBLE); // 0xEFB=1,is_car_convertible %1d%
			CLEO_RegisterOpcode(0xEFC, GET_CAR_VALUE); // 0xEFC=2,get_car_value %1d% store_to %2d%
			CLEO_RegisterOpcode(0xEFD, GET_CAR_PEDALS); // 0xEFD=3,get_car_pedals %1d% gas_to %2d% break_to %3d% 
			CLEO_RegisterOpcode(0xEFE, GET_LOADED_LIBRARY); // 0xEFE=2,get_loaded_library %1d% store_to %2d%
			CLEO_RegisterOpcode(0xF0A, RETURN_TIMES); // 0xF0A=1,return_times %1d%
			CLEO_RegisterOpcode(0xF0E, GET_THIRD_PERSON_CAMERA_TARGET); // 0xF0E=10,get_third_person_camera_target %1d% from %2d% %3d% %4d% start_to %5d% %6d% %7d% end_to %8d% %9d% %10d%
			CLEO_RegisterOpcode(0xF0F, GET_DISTANCE_MULTIPLIER); // 0xF0F=2,get_distance_multiplier %1d% %2d%
			CLEO_RegisterOpcode(0xF10, GET_ACTIVE_CAMERA_ROTATION); // 0xF10=3,get_active_camera_rotation %1d% %2d% %3d%
			CLEO_RegisterOpcode(0xF11, GET_CLOSEST_WATER_DISTANCE); // 0xF11=2,get_closest_water_distance %1d% %2d%
			CLEO_RegisterOpcode(0xF12, GET_CAMERA_STRUCT); // 0xF12=2,get_camera_struct %1d% %2d%
			CLEO_RegisterOpcode(0xF14, GET_CAMERA_ROTATION_INPUT_VALUES); // 0xF14=2,get_camera_rotation_input_values %1d% %2d%
			CLEO_RegisterOpcode(0xF15, SET_CAMERA_ROTATION_INPUT_VALUES); // 0xF15=2,set_camera_rotation_input_values %1d% %2d%
			CLEO_RegisterOpcode(0xF16, SET_ON_MISSION); // 0xF16=1,set_on_mission %1d%
			CLEO_RegisterOpcode(0xF17, GET_MODEL_NAME_POINTER); // 0xF17=2,get_model_name_pointer %1d% to %2s%
			if (!newOpcodesInstalled)
			{
				CLEO_RegisterOpcode(0xD0F, SET_CAR_MODEL_ALPHA); // 0xD0F=2,set_car_model_alpha %1d% alpha %2d%
				CLEO_RegisterOpcode(0xD10, SET_CHAR_MODEL_ALPHA); // 0xD10=2,set_char_model_alpha %1d% alpha %2d%
				CLEO_RegisterOpcode(0xD11, SET_OBJECT_MODEL_ALPHA); // 0xD11=2,set_object_model_alpha %1d% alpha %2d%
				CLEO_RegisterOpcode(0xD2D, GET_LOCAL_TIME); // 0xD2D=8,get_local_time_year_to %1d% month_to %2d% day_of_week_to %3d% day_to %4d% hour_to %5d% minute_to %6d% second_to %7d% milliseconds_to %8d%
				CLEO_RegisterOpcode(0xD2E, SET_SCRIPT_VAR); // 0xD2E=3,set_script %1d% var %2d% value %3d%
				CLEO_RegisterOpcode(0xD2F, GET_SCRIPT_VAR); // 0xD2F=3,%3d% = get_script %1d% var %2d% // keep NewOpcodes order
				CLEO_RegisterOpcode(0xD33, SET_CAR_DOOR_WINDOW_STATE); // 0xD33=3,set_car %1d% door %2d% window_state %3d%
			}

			// Drawing
			CLEO_RegisterOpcode(0xE1E, DRAW_TEXTURE_PLUS); // 0xE1E=15,draw_texture_plus %1d% event %2d% pos %3d% %4d% size %5d% %6d% angle %7d% depth %8d% fix_aspect_ratio %9d% maskTrisCount %10d% maskTrisArray %11d% rgba %12d% %13d% %14d% %15d% 
			CLEO_RegisterOpcode(0xE3C, GET_TEXTURE_FROM_SPRITE); // 0xE3C=2,get_texture_from_sprite %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE62, DRAW_STRING); // 0xE62=8,print %1s% event %2d% at %3d% %4d% scale %5d% %6d% fixAR %7d% style %8d%
			CLEO_RegisterOpcode(0xE63, DRAW_STRING_EXT); // 0xE63=27,print %1s% event %2d% at %3d% %4d% scale %5d% %6d% fixAR %7d% style %8d% prop %9d% align %10d% wrap %11d% justify %12d% color %13d% %14d% %15d% %16d% outline %17d% shadow %18d% dropColor %19d% %20d% %21d% %22d% background %23d% backColor %24d% %25d% %26d% %27d%
			
			// Math
			if (!newOpcodesInstalled) {
				CLEO_RegisterOpcode(0xD1E, QUAT_SLERP); // 0xD1E=4,quat_slerp %1d% to %2d% lambda %3d% result %4d%
				CLEO_RegisterOpcode(0xD16, SET_MATRIX_ROTATION_FROM_QUAT); // 0xD16=2,set_matrix_rotation_from_quat %1d% rotation_from_quat %2d%
				CLEO_RegisterOpcode(0xD17, SET_QUAT_FROM_MATRIX); // 0xD17=2,set_quat_from_matrix %1d% to_quat %2d%
				CLEO_RegisterOpcode(0xD18, ROTATE_QUAT_ON_AXIS); // 0xD18=6,rotate_quat_on_axis %1d% axis_vector %2d% %3d% %4d% angle %5d% combine_op %6d%
				CLEO_RegisterOpcode(0xD19, GET_NORMALISED_QUAT); // 0xD19=2,get_normalised_quat %1d% to_quat %2d%
				CLEO_RegisterOpcode(0xD1A, MULTIPLY_QUATS); // 0xD1A=3,multiply_quats %3d% = quat %1d% * quat %2d% // keep NewOpcodes order
				CLEO_RegisterOpcode(0xD24, INITIALISE_QUAT); // 0xD24=5,set_quat %1d% elements %2d% %3d% %4d% %5d%
				CLEO_RegisterOpcode(0xD29, GET_QUAT_ELEMENTS); // 0xD29=5,get_quat %1d% elements_to %2d% %3d% %4d% %5d%
				CLEO_RegisterOpcode(0xD01, ROTATE_MATRIX_ON_AXIS); // 0xD01=6,rotate_matrix %1d% on_axis %2d% %3d% %4d% angle %5d% combine_op %6d%
				CLEO_RegisterOpcode(0xD02, GET_MATRIX_X_ANGLE); // 0xD02=2,%2d% = matrix %1d% x_angle // keep NewOpcodes order
				CLEO_RegisterOpcode(0xD03, GET_MATRIX_Y_ANGLE); // 0xD03=2,%2d% = matrix %1d% y_angle // keep NewOpcodes order
				CLEO_RegisterOpcode(0xD04, GET_MATRIX_Z_ANGLE); // 0xD04=2,%2d% = matrix %1d% z_angle // keep NewOpcodes order
			}
			CLEO_RegisterOpcode(0xE1F, EASE); // 0xE1F=4,ease %1d% mode %2d% way %3d% to %4d%
			CLEO_RegisterOpcode(0xE27, GET_ANGLE_FROM_TWO_COORDS); // 0xE27=5,get_angle_from_two_coords %1d% %2d% and %3d% %4d% to %5d%
			CLEO_RegisterOpcode(0xE03, PERLIN_NOISE); // 0xE03=2,perlin_noise %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE29, PERLIN_NOISE_FRACTAL); // 0xE29=7,perlin_noise %1d% octaves %2d% frequency %3d% amplitude %4d% lacunarity %5d% persistence %6d% store_to %7d%
			CLEO_RegisterOpcode(0xEF0, GET_COORD_FROM_ANGLED_DISTANCE); // 0xEF0=6,get_coord_from_angled_distance %1d% %2d% angle %3d% dist %4d% store_to %5d% %6d%
			CLEO_RegisterOpcode(0xEF1, PERLIN_NOISE_FRACTAL_2D); // 0xEF1=8,perlin_noise_fractal_2d x %1d% y %2d% octaves %3d% frequency %4d% amplitude %5d% lacunarity %6d% persistence %7d% store_to %8d%
			CLEO_RegisterOpcode(0xEF2, PERLIN_NOISE_FRACTAL_3D); // 0xEF2=9,perlin_noise_fractal_3d x %1d% y %2d% z %3d% octaves %4d% frequency %5d% amplitude %6d% lacunarity %7d% persistence %8d% store_to %9d%
			CLEO_RegisterOpcode(0xEF4, CLAMP_FLOAT); // 0xEF4=4,clamp_float %1d% min %2d% max %3d% store_to %4d%
			CLEO_RegisterOpcode(0xEF7, CLAMP_INT); // 0xEF7=4,clamp_int %1d% min %2d% max %3d% store_to %4d%
			CLEO_RegisterOpcode(0xEB3, CONVERT_DIRECTION_TO_QUAT); // 0xEB3=4,convert_direction_to_quat %1d% dir %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xEF3, LERP); // 0xEF3=4,lerp %1d% %2d% %3d% store_to %4d%
			CLEO_RegisterOpcode(0xF0D, SET_MATRIX_LOOK_DIRECTION); // 0xF0D=7,set_matrix_look_direction %1d% origin %2d% %3d% %4d% dir %5d% %6d% %7d%

			//Bones
			if (!newOpcodesInstalled) {
				CLEO_RegisterOpcode(0xD0B, GET_CHAR_BONE_MATRIX); // 0xD0B=3,get_actor %1d% bone %2d% matrix_to %3d% // IF and SET    
				CLEO_RegisterOpcode(0xD30, GET_CHAR_BONE); // 0xD30=3,%3d% = actor %1d% bone %2d% // IF and SET  
				CLEO_RegisterOpcode(0xD31, GET_BONE_OFFSET_VECTOR); // 0xD31=2,%2d% = bone %1d% offset_vector     
				CLEO_RegisterOpcode(0xD32, GET_BONE_QUAT); // 0xD32=2,%2d% = bone %1d% quat
			}
			
			// Audio
			CLEO_RegisterOpcode(0xE21, GET_AUDIO_SFX_VOLUME); // 0xE21=1,get_audio_sfx_volume %1d%
			CLEO_RegisterOpcode(0xE22, GET_AUDIO_RADIO_VOLUME); // 0xE22=1,get_audio_radio_volume %1d%
			CLEO_RegisterOpcode(0xE3B, GET_AUDIOSTREAM_INTERNAL); // 0xE3B=2,get_audiostream_internal %1d% store_to %2d%

			// Memory
			CLEO_RegisterOpcode(0xE28, WRITE_STRUCT_OFFSET); // 0xE28=4,write_struct %1d% offset %2d% size %3d% value %4d%
			if (!newOpcodesInstalled) {
				CLEO_RegisterOpcode(0xD4E, READ_STRUCT_OFFSET); // 0xD4E=4,%4d% = read_struct %1d% offset %2d% size %3d% // keep newOpcodes ordering for SB
				CLEO_RegisterOpcode(0xD4F, WRITE_STRUCT_OFFSET); // // newOpcodes version of 0E28, the original one doesn't work, this is a fall off to 0E28
				CLEO_RegisterOpcode(0xD27, COPY_MEMORY); // 0xD27=3,copy_memory %1d% to %2d% size %3d%
				CLEO_RegisterOpcode(0xD37, WRITE_STRUCT_PARAM); // 0xD37=3,write_struct_param %1d% param %2d% value %3d%
				CLEO_RegisterOpcode(0xD38, READ_STRUCT_PARAM); // 0xD38=3,%3d% = read_struct_param %1d% param %2d% // keep newOpcodes ordering for SB
			}
			CLEO_RegisterOpcode(0xE6A, MAKE_NOP); // 0xE6A=2,make_nop %1d% size %2d%
			CLEO_RegisterOpcode(0xEE2, READ_STRUCT_OFFSET_MULTI); // 0xEE2=-1,read_struct_offset_multi %1d% offset %2d% total %3d% size %4d%
			CLEO_RegisterOpcode(0xEE3, WRITE_STRUCT_OFFSET_MULTI); // 0xEE3=-1,write_struct_offset_multi %1d% offset %2d% count %3d% size %4d%
			
			// Radar blips
			CLEO_RegisterOpcode(0xE2A, ADD_CLEO_BLIP); // 0xE2A=9,add_cleo_blip %1d% position %2d% %3d% is_short %4d% RGBA %5d% %6d% %7d% %8d% store_to %9d%
			CLEO_RegisterOpcode(0xE2B, REMOVE_CLEO_BLIP); // 0xE2B=1,remove_cleo_blip %1d%

			// Render object
			CLEO_RegisterOpcode(0xE2E, CREATE_RENDER_OBJECT_TO_CHAR_BONE); // 0xE2E=10,create_render_object_to_char_bone %1d% model %2d% bone %3d% offset %4d% %5d% %6d% rotation %7d% %8d% %9d% store_to %10d%
			CLEO_RegisterOpcode(0xF02, CREATE_RENDER_OBJECT_TO_CHAR_BONE_FROM_SPECIAL); // 0xF02=10,create_render_object_to_char_bone_from_special %1d% special_model %2d% bone %3d% offset %4d% %5d% %6d% rotation %7d% %8d% %9d% scale %10d% %11d% %12d% store_to %13d%
			CLEO_RegisterOpcode(0xF03, CREATE_RENDER_OBJECT_TO_OBJECT); // 0xF03=9,create_render_object_to_object %1d% model %2d% offset %3d% %4d% %5d% rotation %6d% %7d% %8d% store_to %9d%
			CLEO_RegisterOpcode(0xF04, CREATE_RENDER_OBJECT_TO_OBJECT_FROM_SPECIAL); // 0xF04=9,create_render_object_to_object_from_special %1d% special_model %2d% offset %3d% %4d% %5d% rotation %6d% %7d% %8d% store_to %9d%
			CLEO_RegisterOpcode(0xE2F, DELETE_RENDER_OBJECT); // 0xE2F=1,delete_render_object %1d%
			CLEO_RegisterOpcode(0xE30, SET_RENDER_OBJECT_AUTO_HIDE); // 0xE30=4,set_render_object_auto_hide %1d% dead %2d% weapon %3d% car %4d%
			CLEO_RegisterOpcode(0xE31, SET_RENDER_OBJECT_VISIBLE); // 0xE31=2,set_render_object_visible %1d% %2d%
			CLEO_RegisterOpcode(0xE35, SET_RENDER_OBJECT_POSITION); // 0xE35=4,set_render_object_position %1d% %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xE36, SET_RENDER_OBJECT_ROTATION); // 0xE36=4,set_render_object_rotation %1d% %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xE37, SET_RENDER_OBJECT_SCALE); // 0xE37=4,set_render_object_scale %1d% %2d% %3d% %4d%
			CLEO_RegisterOpcode(0xE3A, SET_RENDER_OBJECT_DISTORTION); // 0xE3A=5,set_render_object_distortion %1d% %2d% %3d% %4d% %5d%
			
			// Pickup
			CLEO_RegisterOpcode(0xE33, GET_PICKUP_THIS_COORD); // 0xE33=5,get_pickup_this_coord %1d% %2d% %3d% only_available %4d% store_to %5d%
			CLEO_RegisterOpcode(0xE34, GET_PICKUP_MODEL); // 0xE34=2,get_pickup_model %1d% %2d%
			CLEO_RegisterOpcode(0xE38, GET_PICKUP_POINTER); // 0xE38=2,get_pickup_pointer %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE39, GET_PICKUP_TYPE); // 0xE39=2,get_pickup_type %1d% store_to %2d%
			
			// Timer
			CLEO_RegisterOpcode(0xE4E, DISPLAY_ONSCREEN_TIMER_LOCAL); // 0xE4E=2,display_onscreen_timer_local %1d% direction %2d%
			CLEO_RegisterOpcode(0xE4F, DISPLAY_ONSCREEN_TIMER_WITH_STRING_LOCAL); // 0xE4F=3,display_onscreen_timer_with_string_local %1d% direction %2d% GXT %3d%
			CLEO_RegisterOpcode(0xE50, DISPLAY_ONSCREEN_COUNTER_LOCAL); // 0xE50=2,display_onscreen_counter_local %1d% direction %2d%
			CLEO_RegisterOpcode(0xE51, DISPLAY_ONSCREEN_COUNTER_WITH_STRING_LOCAL); // 0xE51=3,display_onscreen_counter_with_string_local %1d% direction %2d% GXT %3d%
			CLEO_RegisterOpcode(0xE52, DISPLAY_TWO_ONSCREEN_COUNTERS_LOCAL); // 0xE52=2,display_two_onscreen_counters_local %1d% max_value %2d%
			CLEO_RegisterOpcode(0xE53, DISPLAY_TWO_ONSCREEN_COUNTERS_WITH_STRING_LOCAL); // 0xE53=3,display_two_onscreen_counters_with_string_local %1d% max_value %2d% GXT %3d%
			CLEO_RegisterOpcode(0xE54, CLEAR_ONSCREEN_TIMER_LOCAL); // 0xE54=1,clear_onscreen_timer_local %1d%
			CLEO_RegisterOpcode(0xE55, CLEAR_ONSCREEN_COUNTER_LOCAL); // 0xE55=1,clear_onscreen_counter_local %1d%
			CLEO_RegisterOpcode(0xE56, SET_ONSCREEN_COUNTER_FLASH_WHEN_FIRST_DISPLAYED_LOCAL); // 0xE56=2,set_onscreen_counter_flash_when_first_displayed_local %1d% flash %2d%
			CLEO_RegisterOpcode(0xE57, SET_TIMER_BEEP_COUNTDOWN_TIME_LOCAL); // 0xE57=2,set_timer_beep_countdown_time_local %1d% secs %2d%
			CLEO_RegisterOpcode(0xE58, SET_ONSCREEN_COUNTER_COLOUR_LOCAL); // 0xE58=2,set_onscreen_counter_colour_local %1d% color %2d%

			// Intelligence
			CLEO_RegisterOpcode(0xE42, IS_CHAR_DOING_TASK_ID); // 0xE42=2,is_char_doing_task_id %1d% %2d%
			CLEO_RegisterOpcode(0xE43, GET_CHAR_TASK_POINTER_BY_ID); // 0xE43=3,get_char_task_pointer_by_id %1d% %2d% store_to %3d%
			CLEO_RegisterOpcode(0xE44, GET_CHAR_KILL_TARGET_CHAR); // 0xE44=2,get_char_kill_target_char %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE46, IS_CHAR_USING_GUN); // 0xE46=1,is_char_using_gun %1d%
			CLEO_RegisterOpcode(0xE47, IS_CHAR_FIGHTING); // 0xE47=1,is_char_fighting %1d%
			CLEO_RegisterOpcode(0xE48, IS_CHAR_FALLEN_ON_GROUND); // 0xE48=1,is_char_fallen_on_ground %1d%
			CLEO_RegisterOpcode(0xE49, IS_CHAR_ENTERING_ANY_CAR); // 0xE49=1,is_char_entering_any_car %1d%
			CLEO_RegisterOpcode(0xE4A, IS_CHAR_EXITING_ANY_CAR); // 0xE4A=1,is_char_exiting_any_car %1d%
			CLEO_RegisterOpcode(0xE4B, IS_CHAR_PLAYING_ANY_SCRIPT_ANIMATION); // 0xE4B=2,is_char_playing_any_script_animation %1d% include_anims %2d%
			CLEO_RegisterOpcode(0xE4C, IS_CHAR_DOING_ANY_IMPORTANT_TASK); // 0xE4C=2,is_char_doing_any_important_task %1d% include_anims %2d%
			CLEO_RegisterOpcode(0xE96, CLEAR_CHAR_PRIMARY_TASKS); // 0xE96=1,clear_char_primary_tasks %1d%
			CLEO_RegisterOpcode(0xE97, CLEAR_CHAR_SECONDARY_TASKS); // 0xE97=1,clear_char_secondary_tasks %1d%
			CLEO_RegisterOpcode(0xEFF, GET_CHAR_SIMPLEST_ACTIVE_TASK); // 0xEFF=3,get_char_simplest_active_task %1d% id_to %2d% pointer_to %3d%

			// Colpoint
			if (!newOpcodesInstalled)
			{
				CLEO_RegisterOpcode(0xD3A, GET_COLLISION_BETWEEN_POINTS); // 0xD3A=20,get_collision_between_points %1d% %2d% %3d% and %4d% %5d% %6d% flags %7d% %8d% %9d% %10d% %11d% %12d% %13d% %14d% ignore_entity %15d% store_point_to %17d% %18d% %19d% entity_to %20d% colpoint_data_to %16d% // keep NewOpcodes order
				CLEO_RegisterOpcode(0xD3B, GET_COLPOINT_NORMAL_VECTOR); // 0xD3B=4,get_colpoint_normal_vector %1d% store_to %2d% %3d% %4d%
				CLEO_RegisterOpcode(0xD3C, GET_COLPOINT_SURFACE); // 0xD3C=2,get_colpoint_surface %1d% store_to %2d%
				CLEO_RegisterOpcode(0xD3E, GET_COLPOINT_DEPTH); // 0xD3E=2,get_colpoint_depth %1d% store_to %2d%
			}
			CLEO_RegisterOpcode(0xE6B, GET_COLPOINT_LIGHTING); // 0xE6B=3,get_colpoint_lighting %1d% from_night %2d% store_to %3d%
			CLEO_RegisterOpcode(0xEE1, GET_COLPOINT_COORDINATES); // 0xEE1=4,get_colpoint_coordinates %1d% store_to %2d% %3d% %4d%

			// List
			CLEO_RegisterOpcode(0xE72, CREATE_LIST); // 0xE72=2,create_list %1d% store_to %2d%
			CLEO_RegisterOpcode(0xE73, DELETE_LIST); // 0xE73=1,delete_list %1d%
			CLEO_RegisterOpcode(0xE74, LIST_ADD); // 0xE74=2,list_add %1d% value %2d%
			CLEO_RegisterOpcode(0xE75, LIST_REMOVE_VALUE); // 0xE75=2,list_remove_value %1d% value %2d%
			CLEO_RegisterOpcode(0xE76, LIST_REMOVE_INDEX); // 0xE76=2,list_remove_index %1d% index %2d%
			CLEO_RegisterOpcode(0xE77, GET_LIST_SIZE); // 0xE77=2,get_list_size %1d% size %2d%
			CLEO_RegisterOpcode(0xE78, GET_LIST_VALUE_BY_INDEX); // 0xE78=3,get_list_value_by_index %1d% index %2d% store_to %3d%
			CLEO_RegisterOpcode(0xE79, RESET_LIST); // 0xE79=1,reset_list %1d%
			CLEO_RegisterOpcode(0xE7A, GET_LIST_STRING_VALUE_BY_INDEX); // 0xE7A=3,get_list_string_value_by_index %1d% index %2d% store_to %3d%
			CLEO_RegisterOpcode(0xE7B, LIST_ADD_STRING); // 0xE7B=2,list_add_string %1d% value %2d%
			CLEO_RegisterOpcode(0xE7C, LIST_REMOVE_STRING_VALUE); // 0xE7C=2,list_remove_string_value %1d% value %2d%
			CLEO_RegisterOpcode(0xE7D, LIST_REMOVE_INDEX_RANGE); // 0xE7D=3,list_remove_index %1d% start %2d% end %3d%
			CLEO_RegisterOpcode(0xE7E, REVERSE_LIST); // 0xE7E=1,reverse_list %1d%
			CLEO_RegisterOpcode(0xF06, REPLACE_LIST_VALUE_BY_INDEX); // 0xF06=3,replace_list_value_by_index %1d% index %2d% value %3d%
			CLEO_RegisterOpcode(0xF07, REPLACE_LIST_STRING_VALUE_BY_INDEX); // 0xF07=3,replace_list_string_value_by_index %1d% index %2d% value %3d%
			CLEO_RegisterOpcode(0xF08, INSERT_LIST_VALUE_BY_INDEX); // 0xF08=3,insert_list_value_by_index %1d% index %2d% value %3d%
			CLEO_RegisterOpcode(0xF09, INSERT_LIST_STRING_VALUE_BY_INDEX); // 0xF09=3,insert_list_string_value_by_index %1d% index %2d% value %3d%

			// Special Models
			CLEO_RegisterOpcode(0xF00, LOAD_SPECIAL_MODEL); // 0xF00=3,load_special_model_dff %1s% txd %2s% store_to %3d%
			CLEO_RegisterOpcode(0xF01, REMOVE_SPECIAL_MODEL); // 0xF01=1,remove_special_model %1d%
			CLEO_RegisterOpcode(0xF05, GET_SPECIAL_MODEL_DATA); // 0xF05=4,get_special_model_data %1d% clump_to %2d% atomic_to %3d% txd_index_to %4d%
			 

			//------------------------------------------------------------------------------------
			
			// Cache addresses (for better mod compatibility)
			maxDffFiles = ReadMemory<int>(0x407104 + 2, true);
			if (maxDffFiles < 10000) maxDffFiles = 20000; //fix if something is wrong

			defaultMouseAccelHorizontalAddress = ReadMemory<uintptr_t>(0x50FB16 + 2, true);
			defaultMouseAccelVerticalAddress = ReadMemory<uintptr_t>(0x50FB26 + 2, true);
			defaultCameraMove00125 = ReadMemory<uintptr_t>(0x50FB10 + 2, true);
			streamingInfoForModel = ReadMemory<CStreamingInfo*>(0x409D42 + 3, true);
			addPedModelAddress = ReadMemory<uintptr_t>(0x5B74A7 + 1, true) + (0x5B74A7 + 5);
			ScriptEffectSystemArray = ReadMemory<tScriptEffectSystem*>(0x483A07 + 3, true);

			scriptSpace = ReadMemory<char*>(0x5D5380, true);
			controllerMode = ReadMemory<uint8_t>(0x47F399, true);

			ARwidthFromWF = ReadMemory<float*>(0x58603F + 2, true);
			ARheightFromWF = ReadMemory<float*>(0x586058 + 2, true);

			totalZonesToReveal = ReadMemory<uint8_t>(0x583B12, true);
			radarBlipSprites = ReadMemory<CSprite2d*>(0x5827EB, true);

			defaultBlipSize = ReadMemory<float*>(0x58605E + 2, true);

			blipsAlwaysRevealed = (inSAMP || ReadMemory<uint8_t>(0x586F17, true) == 0x90);

			startPickups = ReadMemory<uintptr_t>(0x48ADC3, true);
			endPickups = ReadMemory<uintptr_t>(0x4590E4, true);
			sizeOfCPickup = ReadMemory<uint8_t>(0x4590DF + 2, true);
			sizePickups = (endPickups - (uintptr_t)startPickups) / sizeOfCPickup;

			CTheScripts__ScriptConnectLodsObjects = ReadMemory<int*>(0x5D5225 + 1, true);
			sizeScriptConnectLodsObjects = ReadMemory<uint32_t>(0x5D522A + 1, true) * 2;

			ApplyPatches();

			// ----------------------------------------------------------------------------------------

			// VehiclePreRender
			vehiclePreRenderEvent += [](CVehicle* vehicle)
			{
				if (scriptEvents[ScriptEvent::List::CarProcess].size() > 0) {
					int ref = CPools::GetVehicleRef(vehicle);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CarProcess]) scriptEvent->RunScriptEvent(ref);
				}
			};
			beforeGameProcess += []
			{
				// clear stuff that failed to be drawn
				for (unsigned int i = 0; i < DrawEvent::TOTAL_DRAW_EVENT; ++i) {
					ClearMySprites(sprites[i]);
					textDrawer[i].ClearAll();
				}
				if (scriptEvents[ScriptEvent::List::BeforeGameProcess].size() > 0) {
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::BeforeGameProcess]) scriptEvent->RunScriptEvent();
				}
				// Reset ped data per frame
				auto& pedsPool = CPools::ms_pPedPool;
				for (int index = 0; index < pedsPool->m_nSize; ++index)
				{
					if (CPed* ped = pedsPool->GetAt(index))
					{
						// Call char process script event if invisible, because our ped render hook will not call it
						if (ped->m_nPedState == 50) {
							if (scriptEvents[ScriptEvent::List::CharProcess].size() > 0) {
								int ref = CPools::GetPedRef(ped);
								for (auto scriptEvent : scriptEvents[ScriptEvent::List::CharProcess]) scriptEvent->RunScriptEvent(ref);
							}
						}

						PedExtended& xdata = extData.Get(ped);
						if (&xdata != nullptr)
						{
							// Reset AI flags
							xdata.aiFlagsIntValue = 0;

							// Cache tasks
							int activeTaskIndex = 0;
							if (ped->m_pIntelligence)
							{
								CTaskManager* taskMgr = &ped->m_pIntelligence->m_TaskMgr;
								for (unsigned int i = 0; i < 5; i++)
								{
									CTask* task = taskMgr->m_aPrimaryTasks[i];
									while (task)
									{
										CacheOnePedTask(ped, xdata, activeTaskIndex, task, false);
										task = task->GetSubTask();
									}
								}

								for (unsigned int i = 0; i < 5; i++)
								{
									CTask* task = taskMgr->m_aSecondaryTasks[i];
									while (task)
									{
										CacheOnePedTask(ped, xdata, activeTaskIndex, task, true);
										task = task->GetSubTask();
									}
								}
							}
							if (activeTaskIndex < 31) xdata.activeTasks[activeTaskIndex] = -1; // set terminator
						}
					}
				}
			};

			Events::processScriptsEvent.after += []
			{
				// Reset ped data per frame
				auto& pedsPool = CPools::ms_pPedPool;
				for (int index = 0; index < pedsPool->m_nSize; ++index)
				{
					if (CPed* ped = pedsPool->GetAt(index))
					{
						PedExtended& xdata = extData.Get(ped);
						if (&xdata != nullptr) {
							// Reset last damage;
							xdata.lastDamageEntity = nullptr;
							xdata.lastDamageWeapon = 0;
							xdata.lastDamagePart = 0;
							xdata.lastDamageIntensity = 0.0f;
						}
					}
				}
				// Reset vehicle data per frame
				auto& vehsPool = CPools::ms_pVehiclePool;
				for (int index = 0; index < vehsPool->m_nSize; ++index)
				{
					if (CVehicle* vehicle = vehsPool->GetAt(index))
					{
						VehExtended& xdata = vehExtData.Get(vehicle);

						if (&xdata != nullptr) {
							// Reset last damage;
							xdata.lastDamagePed = nullptr;
							xdata.lastDamageType = 0;
							xdata.lastDamageIntensity = 0.0f;
						}
					}
				}
				controllerMode = ReadMemory<uint8_t>(0x47F399, false);
			};

			afterGameProcess += []
			{
				if (!CTimer::m_UserPause && !CTimer::m_CodePause) {
					pausedLastFrame = false;
				}

				// Update camera rotation disable.
				if (disableCamControl)
				{
					// Disables it each frame to make it compatible with GTA V Hud.
					for (unsigned int i = 0; i < disableCamMoveAddressesCount; ++i)
					{
						patch::SetPointer(disableCamMoveAddresses[i], &fZero);
					}
					for (unsigned int i = 0; i < disableCamMoveAddresses00125Count; ++i)
					{
						patch::SetPointer(disableCamMoveAddresses00125[i], &fZero);
					}
					disabledCamControlLastFrame = true;
				}
				else if (disabledCamControlLastFrame)
				{
					// Get using address (to make it compatible with other mods)
					for (unsigned int i = 0; i < disableCamMoveAddressesCount; ++i)
					{
						patch::SetInt(disableCamMoveAddresses[i], (i > 4) ? defaultMouseAccelHorizontalAddress : defaultMouseAccelVerticalAddress);
					}
					for (unsigned int i = 0; i < disableCamMoveAddresses00125Count; ++i)
					{
						patch::SetInt(disableCamMoveAddresses00125[i], defaultCameraMove00125);
					}
					disabledCamControlLastFrame = false;
				}

				if (scriptEvents[ScriptEvent::List::AfterGameProcess].size() > 0) {
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::AfterGameProcess]) scriptEvent->RunScriptEvent();
				}
			};

			// EntityPreRender
			// PATCH MOVED TO: PatchBuildingProcessIfNeeded Events.cpp

			// PedRender, but if pedState is 50 isn't called, so we call it in script process
			pedPreRender += [](CPed* ped)
			{
				if (scriptEvents[ScriptEvent::List::CharProcess].size() > 0) {
					int ref = CPools::GetPedRef(ped);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CharProcess]) scriptEvent->RunScriptEvent(ref);
				}
			};

			entityRenderEffects += [](CEntity* entity) {

				if (entity->m_nType == eEntityType::ENTITY_TYPE_OBJECT) {
					CObject* object = reinterpret_cast<CObject*>(entity);
					ObjExtended& xdata = objExtData.Get(object);
					if (&xdata != nullptr && xdata.renderObjects.size() > 0)
					{
						CMatrixLink* matrix = object->GetMatrix();
						if (matrix) {
							for (RenderObject* renderObject : xdata.renderObjects)
							{
								if (renderObject->isVisible)
								{
									if (renderObject->hideIfDead && object->m_fHealth <= 0.0f) continue;

									RwFrame* frame = renderObject->frame;
									RpAtomic* atomic = renderObject->atomic;
									RpClump* clump;

									if (renderObject->specialModel && renderObject->specialModel->clump) {
										clump = renderObject->specialModel->clump;
									}
									else {
										clump = renderObject->clump;
									}

									if (clump)
									{
										frame = (RwFrame*)clump->object.parent;
									}

									memcpy(&frame->modelling, matrix, sizeof(frame->modelling));
									RwV3d pointsIn = { renderObject->offset.x, renderObject->offset.y, renderObject->offset.z };
									RwV3dTransformPoints(&pointsIn, &pointsIn, 1, (RwMatrix*)matrix);
									frame->modelling.pos = pointsIn;
									if (renderObject->rot.x != 0.0f || renderObject->rot.y != 0.0f || renderObject->rot.z != 0.0f)
									{
										RwFrameRotate(frame, (RwV3d*)0x008D2E00, renderObject->rot.x, rwCOMBINEPRECONCAT);
										RwFrameRotate(frame, (RwV3d*)0x008D2E0C, renderObject->rot.y, rwCOMBINEPRECONCAT);
										RwFrameRotate(frame, (RwV3d*)0x008D2E18, renderObject->rot.z, rwCOMBINEPRECONCAT);
									}
									if (renderObject->scale.x != 1.0f || renderObject->scale.y != 1.0f || renderObject->scale.z != 1.0f)
									{
										RwV3d sizeVector = { renderObject->scale.x, renderObject->scale.y, renderObject->scale.z };
										RwFrameScale(frame, &sizeVector, RwOpCombineType::rwCOMBINEPRECONCAT);
									}
									if (renderObject->dist.x != 0.0f) frame->modelling.at.x = renderObject->dist.x;
									if (renderObject->dist.y != 0.0f) frame->modelling.at.z = renderObject->dist.y;
									if (renderObject->dist.z != 0.0f) frame->modelling.pos.x = renderObject->dist.z; //bug?
									if (renderObject->dist.w != 0.0f) frame->modelling.pos.y = renderObject->dist.w; //bug?
									RwFrameUpdateObjects(frame);

									if (clump)
									{
										RpClumpRender(clump);
									}
									else
									{
										if (atomic) atomic->renderCallBack(atomic);
									}
								}
							}
						}
					}
				}

			};

			Events::pedRenderEvent.after += [](CPed* ped)
			{
				PedExtended& xdata = extData.Get(ped);

				// -- Render objects
				if (ped->m_bIsVisible && ped->m_pRwObject)
				{
					if (&xdata != nullptr && xdata.renderObjects.size() > 0)
					{
						for (RenderObject* renderObject : xdata.renderObjects)
						{
							if (renderObject->isVisible)
							{
								if (renderObject->hideIfCar && ped->m_nPedFlags.bInVehicle) continue;
								if (renderObject->hideIfDead && ped->m_fHealth <= 0.0f) continue;
								if (renderObject->hideIfWeapon && ped->m_nActiveWeaponSlot > 0) continue;

								RwFrame* frame = renderObject->frame;
								RpAtomic* atomic = renderObject->atomic;
								RpClump* clump;

								if (renderObject->specialModel && renderObject->specialModel->clump) {
									clump = renderObject->specialModel->clump;
								}
								else {
									clump = renderObject->clump;
								}

								if (clump)
								{
									frame = (RwFrame*)clump->object.parent;
								}

								RpHAnimHierarchy* hAnimHier = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
								RwMatrix* boneMatrix = &RpHAnimHierarchyGetMatrixArray(hAnimHier)[RpHAnimIDGetIndex(hAnimHier, renderObject->boneId)];
								memcpy(&frame->modelling, boneMatrix, sizeof(frame->modelling));
								RwV3d pointsIn = { renderObject->offset.x, renderObject->offset.y, renderObject->offset.z };
								RwV3dTransformPoints(&pointsIn, &pointsIn, 1, boneMatrix);
								frame->modelling.pos = pointsIn;
								if (renderObject->rot.x != 0.0f || renderObject->rot.y != 0.0f || renderObject->rot.z != 0.0f)
								{
									RwFrameRotate(frame, (RwV3d*)0x008D2E00, renderObject->rot.x, rwCOMBINEPRECONCAT);
									RwFrameRotate(frame, (RwV3d*)0x008D2E0C, renderObject->rot.y, rwCOMBINEPRECONCAT);
									RwFrameRotate(frame, (RwV3d*)0x008D2E18, renderObject->rot.z, rwCOMBINEPRECONCAT);
								}
								if (renderObject->scale.x != 1.0f || renderObject->scale.y != 1.0f || renderObject->scale.z != 1.0f)
								{
									RwV3d sizeVector = { renderObject->scale.x, renderObject->scale.y, renderObject->scale.z };
									RwFrameScale(frame, &sizeVector, RwOpCombineType::rwCOMBINEPRECONCAT);
								}
								if (renderObject->dist.x != 0.0f) frame->modelling.at.x = renderObject->dist.x;
								if (renderObject->dist.y != 0.0f) frame->modelling.at.z = renderObject->dist.y;
								if (renderObject->dist.z != 0.0f) frame->modelling.pos.x = renderObject->dist.z; //bug?
								if (renderObject->dist.w != 0.0f) frame->modelling.pos.y = renderObject->dist.w; //bug?
								RwFrameUpdateObjects(frame);

								if (clump)
								{
									RpClumpRender(clump);
								}
								else
								{
									if (atomic) atomic->renderCallBack(atomic);
								}
							}
						}
					}

				}
			};

			startSaveGame += []
			{
				currentSaveSlot = FrontEndMenuManager.m_nSelectedSaveGame;
				for (auto scriptEvent : scriptEvents[ScriptEvent::List::SaveConfirmation]) scriptEvent->RunScriptEvent(currentSaveSlot + 1);
			};

			loadingEvent += []
			{
				currentSaveSlot = FrontEndMenuManager.m_nSelectedSaveGame;
			};

			// Fixes corrupted old saves with deleted LOD objects
			loadingEventAfterPoolsLoaded.before += []
			{
				for (unsigned int i = 0; i < sizeScriptConnectLodsObjects; ++i)
				{
					if (CTheScripts__ScriptConnectLodsObjects[i] != -1)
					{
						CObject* obj = CPools::GetObject(CTheScripts__ScriptConnectLodsObjects[i]);
						if ((uintptr_t)obj < 0x00400000)
						{
							CTheScripts__ScriptConnectLodsObjects[i] = -1;
						}
					}
				}
			};

			newGameFirstStartEvent += []
			{
				currentSaveSlot = -1; // new game (loadingEvent may change it)
			};

			restartEvent.before += []
			{
				if (currentSaveSlot > -2) { // Fixes a improved fastloader bug
					timesGameRestarted++;
				}
				// Scripts are executed once before restart, so we need to re-execute some events
				for (auto scriptEvent : scriptEvents[ScriptEvent::List::OnMenu]) scriptEvent->RunScriptEvent(true);
				disablePadControl[0] = false;
				disablePadControl[1] = false;
				disablePadControlMovement[0] = false;
				disablePadControlMovement[1] = false;
				disableCamControl = false;
				RadarBlip::Clear();
				ClearScriptLists();
				ScriptEvent::ClearAllScriptEvents();
				for (auto id : specialCharacterModelsUsed) {
					CStreaming::SetMissionDoesntRequireModel(id);
					CStreaming::RemoveModel(id);
				}
				//specialCharacterModelsUsed.clear(); // don't clear it, we use this list to reuse IDs in GET_MODEL_DOESNT_EXIST_IN_RANGE
				for (unsigned int i = 0; i < DrawEvent::TOTAL_DRAW_EVENT; ++i) {
					ClearMySprites(sprites[i]);
					textDrawer[i].ClearAll();
				}
				currentSaveSlot = -1; // new game (loadingEvent may change it now)
			};

			Events::pedDtorEvent.before += [](CPed* ped)
			{
				if (scriptEvents[ScriptEvent::List::CharDelete].size() > 0) {
					int ref = CPools::GetPedRef(ped);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CharDelete]) scriptEvent->RunScriptEvent(ref);
				}
				PedExtended& data = extData.Get(ped);
				if (&data != nullptr && data.renderObjects.size() > 0)
				{
					DeleteAllRenderObjectsFromChar(data);
				}
			};

			Events::vehicleDtorEvent.before += [](CVehicle* vehicle) {
				if (scriptEvents[ScriptEvent::List::CarDelete].size() > 0) {
					int ref = CPools::GetVehicleRef(vehicle);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CarDelete]) scriptEvent->RunScriptEvent(ref);
				}
			};

			Events::objectDtorEvent.before += [](CObject* object) {
				int ref = CPools::GetObjectRef(object);
				if (scriptEvents[ScriptEvent::List::ObjectDelete].size() > 0) {
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::ObjectDelete]) scriptEvent->RunScriptEvent(ref);
				}
				ObjExtended& data = objExtData.Get(object);
				if (&data != nullptr && data.renderObjects.size() > 0)
				{
					DeleteAllRenderObjectsFromObject(data);
				}
				for (unsigned int i = 0; i < sizeScriptConnectLodsObjects; ++i)
				{
					if (CTheScripts__ScriptConnectLodsObjects[i] == ref) CTheScripts__ScriptConnectLodsObjects[i] = -1;
				}
			};

			// Called before menu draw, because drawMenuBackgroundEvent isn't compatible with RenderHook
			onMenu.after += [] {
				for (auto scriptEvent : scriptEvents[ScriptEvent::List::OnMenu]) scriptEvent->RunScriptEvent(!pausedLastFrame);
				if (!pausedLastFrame) {
					pausedLastFrame = true;
				}
				UpdateKeyPresses();
			};

			// Bullet impact event
			patch::RedirectCall(0x73CD92, MyDoBulletImpact);
			patch::RedirectCall(0x741199, MyDoBulletImpact);
			patch::RedirectCall(0x7411DF, MyDoBulletImpact);
			patch::RedirectCall(0x7412DF, MyDoBulletImpact);
			patch::RedirectCall(0x741E30, MyDoBulletImpact);
			/*injector::MakeInline<0x73B57C, 0x73B57C + 7>([](injector::reg_pack& regs)
			{
				// not compatible with Bullet Mod and
				regs.ebx = *(uint32_t*)(regs.esp + 0x90 + 0x4); //mov     ebx, [esp+90h+owner]
				CWeapon *weapon = (CWeapon *)regs.edi;
				CPed *owner = (CPed *)regs.ebx;
				CEntity *victim = *(CEntity**)(regs.esp + 0x90 + 0x8);
				CColPoint *colPoint = *(CColPoint**)(regs.esp + 0x90 + 0x14);

				if (scriptEvents[ScriptEvent::List::BulletImpact].size() > 0) {
					int ownerRef = -1;
					if (owner) ownerRef = CPools::GetPedRef(owner);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::BulletImpact]) scriptEvent->RunScriptEvent((DWORD)ownerRef, (DWORD)victim, (DWORD)weapon->m_nType, (DWORD)colPoint);
				}
			});*/
			// Ped creator (audio initialization)
			injector::MakeInline<0x4E6916, 0x4E6916 + 6>([](injector::reg_pack& regs)
			{
				*(uint8_t*)(regs.esi + 0x99) = 0; //mov [esi+99h], bl
				if (scriptEvents[ScriptEvent::List::CharCreate].size() > 0) {
					CPed* ped = *(CPed**)(regs.esp + 0xC + 0x4);
					int ref = CPools::GetPedRef(ped);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CharCreate]) scriptEvent->RunScriptEvent(ref);
				}
			});

			// Vehicle creator (audio initialization; somewhere in random place because I don't trust other mods to hook at calls, like UsesSiren)
			injector::MakeInline<0x4F7798, 0x4F7798 + 6>([](injector::reg_pack& regs)
			{
				*(uint32_t*)(regs.ebp + 0x180) = regs.ebx; //mov [ebp+180h], ebx
				if (scriptEvents[ScriptEvent::List::CarCreate].size() > 0) {
					CVehicle* vehicle = (CVehicle*)(regs.edx);
					int ref = CPools::GetVehicleRef(vehicle);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CarCreate]) scriptEvent->RunScriptEvent(ref);
				}
			});

			// my objectCtorEvent, called really after
			injector::MakeInline<0x59FB1E, 0x59FB1E + 6>([](injector::reg_pack& regs)
			{
				*(uint32_t*)(regs.esi + 0x174) = regs.ebx; //mov [esi+174h], ebx
				if (scriptEvents[ScriptEvent::List::ObjectCreate].size() > 0) {
					int ref = CPools::GetObjectRef(reinterpret_cast<CObject*>(regs.esi));
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::ObjectCreate]) scriptEvent->RunScriptEvent(ref);
				}
			});

			// Ped damage event
			injector::MakeInline<0x4B5AF9, 0x4B5AF9 + 7>([](injector::reg_pack& regs)
			{
				*(uint32_t*)(regs.ebp + 0x4) = 0; //mov     dword ptr [ebp+4], 0
				CPedDamageResponseCalculator* damageCalculator = (CPedDamageResponseCalculator*)regs.esi;
				//if (damageCalculator-> != 0.0f) {
				CPed* ped = (CPed*)regs.edi;
				PedExtended& data = extData.Get(ped);
				if (&data != nullptr) {
					data.lastDamageEntity = damageCalculator->m_pDamager;
					data.lastDamageIntensity = damageCalculator->m_fDamageFactor;
					data.lastDamageWeapon = damageCalculator->m_weaponType;
					data.lastDamagePart = damageCalculator->m_pedPieceType;
				}

				if (scriptEvents[ScriptEvent::List::CharDamage].size() > 0) {
					int ref = CPools::GetPedRef(ped);
					for (auto scriptEvent : scriptEvents[ScriptEvent::List::CharDamage]) scriptEvent->RunScriptEvent(ref);
				}
				//}
			});

			// Vehicle weapon damage event
			injector::MakeInline<0x6D804F, 0x6D804F + 6>([](injector::reg_pack& regs)
			{
				float intensity = *(float*)(regs.esp + 0x90);
				if (intensity != 0.0f) {
					CVehicle* vehicle = (CVehicle*)regs.esi;
					VehExtended& data = vehExtData.Get(vehicle);
					if (&data != nullptr) {
						data.lastDamagePed = (CEntity*)regs.edi;
						data.lastDamageType = regs.ebx;
						data.lastDamageIntensity = intensity;
					}

					if (scriptEvents[ScriptEvent::List::CarWeaponDamage].size() > 0) {
						int ref = CPools::GetVehicleRef(vehicle);
						for (auto scriptEvent : scriptEvents[ScriptEvent::List::CarWeaponDamage]) scriptEvent->RunScriptEvent(ref);
					}
				}
				float f = *(float*)(regs.esi + 0x4C0); //fld esi+4C0
				asm_fld(f);
			});

			// Ped ignore anims
			injector::MakeInline<0x4B4029, 0x4B4029 + 6>([](injector::reg_pack& regs)
			{
				regs.ecx = *(uint32_t*)(regs.ebx + 0x534); //mov     ecx, [ebx+534h]
				CPed* ped = (CPed*)regs.ebx;
				PedExtended& data = extData.Get(ped);
				if (&data != nullptr && data.ignoreDamageAnims) {
					*(uintptr_t*)(regs.esp - 0x4) = 0x4B4019; // ret
				}
			});

			/*if (gameVersion == GAME_10US_HOODLUM)
			{
				// Avoid deleted script LOD crash (Hoodlum)
				injector::MakeInline<0x156DA35, 0x156DA35 + 6>([](injector::reg_pack& regs)
				{
					//mov     [eax+30h], esi ; mov     al, [esi+34h]
					if (regs.eax < 0x00400000)
					{
						*(uint32_t*)(regs.esp - 0x4) = 0x156DA59;
					}
					else
					{
						*(uint32_t*)(regs.eax + 0x30) = regs.esi;
						regs.eax = *(uint8_t*)(regs.esi + 0x34);
					}
				});
			}
			else
			{
				if (gameVersion == GAME_10US_COMPACT)
				{
					// Avoid deleted script LOD crash (Compact)
					injector::MakeInline<0x470A65, 0x470A65 + 6>([](injector::reg_pack& regs)
					{
						if (regs.eax < 0x00400000)
						{
							*(uint32_t*)(regs.esp - 0x4) = 0x470A89;
						}
						else
						{
							*(uint32_t*)(regs.eax + 0x30) = regs.esi;
							regs.eax = *(uint8_t*)(regs.esi + 0x34);
						}
					});
				}
			}*/

			//-----------------------------------------------------------------------------------------

			Events::drawingEvent.before += [] {
				DrawMySprites(sprites[DrawEvent::BeforeDrawing]);
				ClearMySprites(sprites[DrawEvent::BeforeDrawing]);
				textDrawer[DrawEvent::BeforeDrawing].DrawPrints();
			};

			Events::drawingEvent.after += [] {
				DrawMySprites(sprites[DrawEvent::AfterDrawing]);
				ClearMySprites(sprites[DrawEvent::AfterDrawing]);
				textDrawer[DrawEvent::AfterDrawing].DrawPrints();
			};

			beforeHud += [] {
				DrawMySprites(sprites[DrawEvent::BeforeHud]);
				ClearMySprites(sprites[DrawEvent::BeforeHud]);
				textDrawer[DrawEvent::BeforeHud].DrawPrints();
			};

			afterHud += [] {
				DrawMySprites(sprites[DrawEvent::AfterHud]);
				ClearMySprites(sprites[DrawEvent::AfterHud]);
				textDrawer[DrawEvent::AfterHud].DrawPrints();
			};

			Events::drawRadarEvent.before += [] {
				DrawMySprites(sprites[DrawEvent::BeforeRadar]);
				ClearMySprites(sprites[DrawEvent::BeforeRadar]);
				textDrawer[DrawEvent::BeforeRadar].DrawPrints();
			};

			Events::drawRadarEvent.after += [] {
				DrawMySprites(sprites[DrawEvent::AfterRadar]);
				ClearMySprites(sprites[DrawEvent::AfterRadar]);
				textDrawer[DrawEvent::AfterRadar].DrawPrints();
			};

			Events::drawRadarOverlayEvent.before += [] {
				DrawMySprites(sprites[DrawEvent::BeforeRadarOverlay]);
				ClearMySprites(sprites[DrawEvent::BeforeRadarOverlay]);
				textDrawer[DrawEvent::BeforeRadarOverlay].DrawPrints();
			};

			Events::drawRadarOverlayEvent.after += [] {
				DrawMySprites(sprites[DrawEvent::AfterRadarOverlay]);
				ClearMySprites(sprites[DrawEvent::AfterRadarOverlay]);
				textDrawer[DrawEvent::AfterRadarOverlay].DrawPrints();
			};

			Events::drawBlipsEvent.before += [] {
				DrawMySprites(sprites[DrawEvent::BeforeBlips]);
				ClearMySprites(sprites[DrawEvent::BeforeBlips]);
				textDrawer[DrawEvent::BeforeBlips].DrawPrints();
			};

			Events::drawBlipsEvent.after += [] {
				DrawMySprites(sprites[DrawEvent::AfterBlips]);
				ClearMySprites(sprites[DrawEvent::AfterBlips]);
				textDrawer[DrawEvent::AfterBlips].DrawPrints();
			};

			drawAfterFade.after += [] {
				DrawMySprites(sprites[DrawEvent::AfterFade]);
				ClearMySprites(sprites[DrawEvent::AfterFade]);
				textDrawer[DrawEvent::AfterFade].DrawPrints();

				// It's better to update all keys and buttons after all game processing, to make it compatible with script events
				UpdateKeyPresses();
			};

		};

		// ----------------------------------------------------------------------------------------

		CLEO_AddScriptDeleteDelegate(ScriptDeleteEvent);
    }

	static void __fastcall MyDoBulletImpact(CWeapon* weapon, int i, CEntity* owner, CEntity* victim, CVector* startPoint, CVector* endPoint, CColPoint* colPoint, int a7)
	{
		if (scriptEvents[ScriptEvent::List::BulletImpact].size() > 0) {
			for (auto scriptEvent : scriptEvents[ScriptEvent::List::BulletImpact]) scriptEvent->RunScriptEvent((DWORD)owner, (DWORD)victim, (DWORD)weapon->m_eWeaponType, (DWORD)colPoint);
		}
		weapon->DoBulletImpact(owner, victim, startPoint, endPoint, colPoint, a7);
		return;
	}

	static void UpdateKeyPresses()
	{

		// Update keys pressed
		memset(keysPressedLastFrame, 0, sizeof(keysPressedLastFrame));
		for (unsigned int vk = 0x01; vk < 0xFF; ++vk)
		{
			keysPressedLastFrame[vk] = ((GetKeyState(vk) & 0x8000) != 0);
		}

		// Update buttons pressed, and control disable
		memset(buttonsPressedLastFrame, 0, sizeof(buttonsPressedLastFrame));
		for (unsigned int pad = 0; pad < 2; ++pad)
		{
			for (unsigned int id = 0; id < 20; ++id)
			{
				// There is old state, but isn't array, and also this function may be changed by some crazy script
				buttonsPressedLastFrame[pad][id] = reinterpret_cast<CRunningScript*>(0)->GetPadState(pad, id);
			}
			// Toggle movement only
			if (disablePadControlMovement[pad])
			{
				CPad::GetPad(pad)->DisablePlayerControls |= 1 << 0;
				disabledPadControlMovementLastFrame[pad] = true;
			}
			else if (disabledPadControlMovementLastFrame[pad])
			{
				CPad::GetPad(pad)->DisablePlayerControls &= ~(1 << 0);
				disabledPadControlMovementLastFrame[pad] = false;
			}
			// Toggle controls only
			if (disablePadControl[pad])
			{
				CPad::GetPad(pad)->DisablePlayerControls |= 1 << 1;
				disabledPadControlLastFrame[pad] = true;
			}
			else if (disabledPadControlLastFrame[pad])
			{
				CPad::GetPad(pad)->DisablePlayerControls &= ~(1 << 1);
				disabledPadControlLastFrame[pad] = false;
			}
		}
	}

} cLEOPlus;

extern "C" uint32_t __declspec(dllexport) GetCleoPlusVersion(CVehicle * vehicle)
{
	return CLEOPLUS_VERSION_INT;
}
