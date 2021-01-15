/*
Test codes for CLEO+ by Junior_Djjr - MixMods.com.br
Everything about CLEO+: https://forum.mixmods.com.br/f141-gta3script-cleo/t5206-como-criar-scripts-com-cleoplus
Maybe not all codes here is working, and maybe it's expected, well, welcome to the mess...
*/

SCRIPT_START
{
    LVAR_INT i j k l m n o eventArgVar lCircles
    LVAR_FLOAT g g1 g2 g3 radius
    LVAR_FLOAT x y z x2 y2 z2
    LVAR_INT scplayer obj car char pBuffer
    LVAR_TEXT_LABEL16 string

    IF LOAD_DYNAMIC_LIBRARY "CLEO+.cleo" (i)
        IF GET_DYNAMIC_LIBRARY_PROCEDURE "GetCleoPlusVersion" i (j)
            CALL_FUNCTION_RETURN j 0 0 ()(j)
            IF j < 0x01000600 // 01 00 06 00 = v1.0.6.0
                PRINT_STRING_NOW "~r~Outdated CLEO+ version. Update it." 5000
                FREE_DYNAMIC_LIBRARY i
                TERMINATE_THIS_CUSTOM_SCRIPT
            ENDIF
        ENDIF
        FREE_DYNAMIC_LIBRARY i
    ELSE
        PRINT_STRING_NOW "~r~CLEO+ not installed." 5000
        TERMINATE_THIS_CUSTOM_SCRIPT
    ENDIF
    
    GET_PLAYER_CHAR 0 scplayer

    WHILE TRUE
        WAIT 0

        IF TEST_CHEAT "P1"
            GOSUB Test74
        ENDIF
        IF TEST_CHEAT "P2"
            GOSUB Test71
        ENDIF
        IF TEST_CHEAT "P3"
            GOSUB Test10
        ENDIF
        IF TEST_CHEAT "P4"
            j = 123 // the value will fall off in the first var from our new thread, so, "i" will be 123
            STREAM_CUSTOM_SCRIPT_FROM_LABEL Test47 j
            GET_LAST_CREATED_CUSTOM_SCRIPT j
            PRINT_FORMATTED_NOW "Created script %x" 1000 j
            WAIT 2000
            // now, "i" will be "133"
            GET_THREAD_VAR j 0 (k)
            k += 10
            SET_THREAD_VAR j 0 k
        ENDIF
        IF TEST_CHEAT "P5"
            TERMINATE_THIS_CUSTOM_SCRIPT
        ENDIF
        IF TEST_CHEAT "P6"
            GOSUB Test23
        ENDIF
        IF TEST_CHEAT "P7"
            GOSUB Test73
        ENDIF
        IF TEST_CHEAT "P8"
            GOSUB Test66
        ENDIF

    ENDWHILE

    Test74:
    WHILE NOT TEST_CHEAT "EXIT"
        WAIT 0
        y = 50.0
        DRAW_STRING "DRAW_EVENT_AFTER_BLIPS" DRAW_EVENT_AFTER_BLIPS 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_AFTER_DRAWING" DRAW_EVENT_AFTER_DRAWING 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_AFTER_FADE" DRAW_EVENT_AFTER_FADE 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_AFTER_HUD" DRAW_EVENT_AFTER_HUD 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_AFTER_RADAR" DRAW_EVENT_AFTER_RADAR 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_AFTER_RADAR_OVERLAY" DRAW_EVENT_AFTER_RADAR_OVERLAY 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_BEFORE_BLIPS" DRAW_EVENT_BEFORE_BLIPS 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_BEFORE_DRAWING" DRAW_EVENT_BEFORE_DRAWING 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_BEFORE_HUD" DRAW_EVENT_BEFORE_HUD 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_BEFORE_RADAR" DRAW_EVENT_BEFORE_RADAR 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES
        y += 10.0
        DRAW_STRING "DRAW_EVENT_BEFORE_RADAR_OVERLAY" DRAW_EVENT_BEFORE_RADAR_OVERLAY 100.0 y 0.5 1.0 TRUE FONT_SUBTITLES

        DRAW_TEXTURE_PLUS 0 DRAW_EVENT_AFTER_HUD 320.0 224.0 640.0 448.0 0.0 0.0 FALSE 0 0 255 255 0 100
    ENDWHILE
    RETURN

    Test73:
    PRINT_STRING_NOW "Y to force weapon fire. EXIT to exit" 2000
    SET_SCRIPT_EVENT_CHAR_PROCESS ON SimulateWeaponFire n
    WHILE NOT TEST_CHEAT "EXIT"
        WAIT 0
    ENDWHILE
    SET_SCRIPT_EVENT_CHAR_PROCESS OFF SimulateWeaponFire n
    RETURN

    Test72:
    PRINT_STRING "Use . to pass time" 1000
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        IF IS_KEY_PRESSED VK_OEM_PERIOD
            WHILE IS_KEY_PRESSED VK_OEM_PERIOD
                PASS_TIME 10
                WAIT 100
            ENDWHILE
        ENDIF
    ENDWHILE
    RETURN

    Test71:
    REQUEST_PRIORITY_MODEL 1529 
    LOAD_ALL_PRIORITY_MODELS_NOW
    CREATE_OBJECT_NO_SAVE 1529 0.0 0.0 0.0 FALSE FALSE (k)
    WAIT 500

    GET_OBJECT_POINTER k l
    READ_STRUCT_OFFSET l 0x14 4 (l) //matrix
    
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        
        GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS 0.0 0.0 0.0 (x y z)
        GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS 0.0 20.0 0.0 (x2 y2 z2)

        GET_PED_POINTER scplayer i
        GET_LABEL_POINTER Buffer128 j // at least 44 bytes is required. If you don't send a pointer, it will work without COLPOINT
        IF GET_COLLISION_BETWEEN_POINTS (x y z) (x2 y2 z2) TRUE FALSE FALSE FALSE FALSE TRUE TRUE TRUE i j (x y z i)

            SET_OBJECT_COORDINATES k x y z
        
            GET_COLPOINT_NORMAL_VECTOR j x2 y2 z2

            x2 *= 10.0
            y2 *= 10.0
            z2 *= 10.0

            m = j + 44

            CONVERT_DIRECTION_TO_QUAT m x2 y2 z2

            // WTF?
            ROTATE_QUAT_ON_AXIS m 0.0 0.0 1.0 270.0 1
            ROTATE_QUAT_ON_AXIS m 0.0 1.0 0.0 90.0 1
            ROTATE_QUAT_ON_AXIS m 1.0 0.0 0.0 180.0 1

            SET_MATRIX_ROTATION_FROM_QUAT l m

            //GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS k 0.0 0.1 0.0 x y z
            //SET_OBJECT_COORDINATES k x y z
        ENDIF
    ENDWHILE
    
    RETURN

    Test70:
    IF GET_MODEL_INFO 400 i
        READ_STRUCT_OFFSET i 0x8 2 (j) // m_wUsageCount
        PRINT_FORMATTED_NOW "Model info: %x usage count: %i" 1000 i j
    ELSE
        PRINT_STRING "~r~Fail to get model info" 1000
    ENDIF
    RETURN

    Test69:
    GET_LABEL_POINTER Buffer128 i
    j = i + 8 // param 2
    WRITE_MEMORY j 4 12345 FALSE

    READ_STRUCT_PARAM i 2 (k)
    PRINT_FORMATTED_NOW "%i" 1000 k

    WAIT 1000

    WRITE_STRUCT_PARAM i 2 54321

    j = i + 8 // param 2
    READ_MEMORY j 4 FALSE (k)
    PRINT_FORMATTED_NOW "%i" 1000 k

    RETURN

    Test68:
    IF GET_LOADED_LIBRARY "SAMPFUNCS.asi" (i)
        PRINT_STRING "~r~WRITE_CLIPBOARD_DATA_FROM and READ_CLIPBOARD_DATA_TO are incompatible with SAMPFuncs" 5000
    ELSE
        PRINT_STRING "Use CTRL+C / CTRL+V" 1000
        GET_LABEL_POINTER Buffer128 i

        WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
            WAIT 0

            IF IS_KEY_PRESSED VK_LCONTROL
            AND IS_KEY_PRESSED VK_KEY_C
                COPY_STRING "Test 1" i
                GET_STRING_LENGTH $i (j)
                j++ // null terminator
                IF WRITE_CLIPBOARD_DATA_FROM i j
                    PRINT_STRING "Clipboard set" 500
                ELSE
                    PRINT_STRING "~r~Fail to write clipboard" 2000
                ENDIF
            ENDIF

            IF IS_KEY_PRESSED VK_LCONTROL
            AND IS_KEY_PRESSED VK_KEY_V
                IF READ_CLIPBOARD_DATA_TO i 128 // 128 is the max size of our buffer
                    PRINT_FORMATTED_NOW "'%s'" 2000 $i
                ELSE
                    PRINT_STRING "~r~Fail to read clipboard" 2000
                ENDIF
            ENDIF

        ENDWHILE
    ENDIF
    RETURN

    Test67:
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 2.0 0.0 x y z
    REQUEST_PRIORITY_MODEL 1478 
    LOAD_ALL_PRIORITY_MODELS_NOW
    CREATE_OBJECT_NO_SAVE 1478 x y z FALSE FALSE i
    WAIT 1000
    SIMULATE_OBJECT_DAMAGE i 9999.9 WEAPONTYPE_RUNOVERBYCAR
    MARK_MODEL_AS_NO_LONGER_NEEDED 1478
    REMOVE_MODEL_IF_UNUSED 1478
    RETURN

    Test66:
    k = 2 // octaves
    x2 = 0.03 // frequency
    y2 = 1.0 // amplitude
    z2 = 2.0 // lacunarity
    g1 = 0.5 // persistence

    PRINT_STRING_NOW "~r~Caution! ~s~This is slow due to 100x100 DRAW_TEXTURE_PLUS. Also disable SCRLog." 4000
    
    WHILE NOT TEST_CHEAT "EXIT"
        WAIT 0

        IF IS_KEY_PRESSED VK_KEY_O
            IF IS_KEY_JUST_PRESSED 107
                k++
            ENDIF
            IF IS_KEY_JUST_PRESSED 109
                k--
            ENDIF
            CLAMP_INT k 0 100 (k)
        ENDIF

        IF IS_KEY_PRESSED VK_KEY_F
            IF IS_KEY_PRESSED 107
                x2 +=@ 0.001
            ENDIF
            IF IS_KEY_PRESSED 109
                x2 -=@ 0.001
            ENDIF
            CLAMP_FLOAT x2 0.0 100.0 (x2)
        ENDIF
        
        IF IS_KEY_PRESSED VK_KEY_A
            IF IS_KEY_PRESSED 107
                y2 +=@ 0.1
            ENDIF
            IF IS_KEY_PRESSED 109
                y2 -=@ 0.1
            ENDIF
            CLAMP_FLOAT y2 0.0 100.0 (y2)
        ENDIF
        
        IF IS_KEY_PRESSED VK_KEY_L
            IF IS_KEY_PRESSED 107
                z2 +=@ 0.1
            ENDIF
            IF IS_KEY_PRESSED 109
                z2 -=@ 0.1
            ENDIF
            CLAMP_FLOAT z2 0.0 100.0 (z2)
        ENDIF
        
        IF IS_KEY_PRESSED VK_KEY_P
            IF IS_KEY_PRESSED 107
                g1 +=@ 0.1
            ENDIF
            IF IS_KEY_PRESSED 109
                g1 -=@ 0.1
            ENDIF
            CLAMP_FLOAT g1 0.0 100.0 (g1)
        ENDIF
        
        GET_LABEL_POINTER Buffer128 j

        DRAW_STRING "'EXIT' to exit. Letter -+" DRAW_EVENT_AFTER_HUD 310.0 170.0 0.3 0.6 TRUE FONT_SUBTITLES 

        DRAW_STRING "(O)ctaves" DRAW_EVENT_AFTER_HUD 310.0 180.0 0.3 0.6 TRUE FONT_SUBTITLES 
        STRING_FORMAT j "%i" k
        DRAW_STRING $j DRAW_EVENT_AFTER_HUD 360.0 180.0 0.3 0.6 TRUE FONT_SUBTITLES 
        j += 8

        DRAW_STRING "(F)requency" DRAW_EVENT_AFTER_HUD 310.0 190.0 0.3 0.6 TRUE FONT_SUBTITLES 
        STRING_FORMAT j "%.3f" x2
        DRAW_STRING $j DRAW_EVENT_AFTER_HUD 360.0 190.0 0.3 0.6 TRUE FONT_SUBTITLES 
        j += 8

        DRAW_STRING "(A)mplitude" DRAW_EVENT_AFTER_HUD 310.0 200.0 0.3 0.6 TRUE FONT_SUBTITLES 
        STRING_FORMAT j "%.3f" y2
        DRAW_STRING $j DRAW_EVENT_AFTER_HUD 360.0 200.0 0.3 0.6 TRUE FONT_SUBTITLES 
        j += 8

        DRAW_STRING "(L)acunarity" DRAW_EVENT_AFTER_HUD 310.0 210.0 0.3 0.6 TRUE FONT_SUBTITLES 
        STRING_FORMAT j "%.3f" z2
        DRAW_STRING $j DRAW_EVENT_AFTER_HUD 360.0 210.0 0.3 0.6 TRUE FONT_SUBTITLES 
        j += 8

        DRAW_STRING "(P)persistence" DRAW_EVENT_AFTER_HUD 310.0 220.0 0.3 0.6 TRUE FONT_SUBTITLES 
        STRING_FORMAT j "%.3f" g1
        DRAW_STRING $j DRAW_EVENT_AFTER_HUD 360.0 220.0 0.3 0.6 TRUE FONT_SUBTITLES 
        j += 8

        // draw result loop
        x = 0.0
        y = 0.0
        WHILE TRUE

            PERLIN_NOISE_FRACTAL_2D (x y) k x2 y2 z2 g1 (g)
            g += 0.5
            g *= 128.0
            CLAMP_FLOAT g 0.0 255.0 (g)

            i =# g 

            x += 200.0
            y += 150.0
            DRAW_TEXTURE_PLUS 0 DRAW_EVENT_AFTER_HUD x y 1.0 1.0 0.0 0.0 0 0 0 (i i i 255) 
            x -= 200.0
            y -= 150.0

            // next pixel
            x += 1.0
            IF x > 100.0
                y += 1.0
                x = 0.0
                IF y > 100.0
                    BREAK
                ENDIF
            ENDIF
        ENDWHILE
    ENDWHILE
    RETURN
 
    Test65:
    CREATE_OBJECT_NO_SAVE 1318 0.0 0.0 0.0 FALSE FALSE k // arrow
    g = 0.0
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        STORE_CLOSEST_ENTITIES scplayer i j
        IF j > 0
            GET_CHAR_COORDINATES scplayer x y z 
            GET_CHAR_COORDINATES j x2 y2 z2
            GET_ANGLE_FROM_TWO_COORDS x y x2 y2 g
            GET_COORD_FROM_ANGLED_DISTANCE x y g 1.0 (x2 y2)
            SET_OBJECT_COORDINATES k x2 y2 z2
            SET_OBJECT_SCALE k 0.5
            SET_OBJECT_ROTATION k 90.0 0.0 g
        ELSE
            SET_OBJECT_COORDINATES k 0.0 0.0 0.0
        ENDIF
    ENDWHILE
    DELETE_OBJECT k
    RETURN
 
    Test64:
    PRINT_STRING_NOW "Adding events. Hold action to remove them." 1000
    WAIT 500
    SET_SCRIPT_EVENT_CHAR_PROCESS ON EventCharProcess eventArgVar
    SET_SCRIPT_EVENT_CAR_PROCESS ON EventCarProcess eventArgVar
    SET_SCRIPT_EVENT_OBJECT_PROCESS ON EventObjectProcess eventArgVar
    //SET_SCRIPT_EVENT_BUILDING_PROCESS ON EventBuildingProcess eventArgVar
    timera = 0
    WHILE timera < 2000
        WAIT 0
        IF NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
            timera = 0
        ENDIF
    ENDWHILE
    SET_SCRIPT_EVENT_CHAR_PROCESS OFF EventCharProcess eventArgVar
    SET_SCRIPT_EVENT_CAR_PROCESS OFF EventCarProcess eventArgVar
    SET_SCRIPT_EVENT_OBJECT_PROCESS OFF EventObjectProcess eventArgVar
    //SET_SCRIPT_EVENT_BUILDING_PROCESS OFF EventBuildingProcess eventArgVar
    RETURN

    Test63:
    PRINT_STRING_NOW "Adding events. Hold action to remove them." 1000
    WAIT 500
    SET_SCRIPT_EVENT_CHAR_CREATE ON EventCharCreate_StartInternalThread eventArgVar
    SET_SCRIPT_EVENT_CAR_CREATE ON EventCarCreate_StartInternalThread eventArgVar
    timera = 0
    WHILE timera < 2000
        WAIT 0
        IF NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
            timera = 0
        ENDIF
    ENDWHILE
    SET_SCRIPT_EVENT_CHAR_CREATE OFF EventCharCreate_StartInternalThread eventArgVar
    SET_SCRIPT_EVENT_CAR_CREATE OFF EventCarCreate_StartInternalThread eventArgVar
    RETURN

    Test62:
    PRINT_STRING_NOW "Adding events. Hold action to remove them." 2000
    WAIT 500
    SET_SCRIPT_EVENT_SAVE_CONFIRMATION ON EventSave eventArgVar
    SET_SCRIPT_EVENT_CHAR_CREATE ON EventCharCreate eventArgVar
    SET_SCRIPT_EVENT_CHAR_DELETE ON EventCharDelete eventArgVar
    SET_SCRIPT_EVENT_CAR_CREATE ON EventCarCreate eventArgVar
    SET_SCRIPT_EVENT_CAR_DELETE ON EventCarDelete eventArgVar
    SET_SCRIPT_EVENT_OBJECT_CREATE ON EventObjectCreate eventArgVar
    SET_SCRIPT_EVENT_OBJECT_DELETE ON EventObjectDelete eventArgVar
    SET_SCRIPT_EVENT_ON_MENU ON EventOnMenu eventArgVar
    SET_SCRIPT_EVENT_CHAR_DAMAGE ON EventCharDamage eventArgVar
    SET_SCRIPT_EVENT_CAR_WEAPON_DAMAGE ON EventCarDamage eventArgVar
    SET_SCRIPT_EVENT_BULLET_IMPACT ON EventBulletImpact i j k l
    timera = 0
    WHILE timera < 1500
        WAIT 0
        IF NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
            timera = 0
        ENDIF
    ENDWHILE
    SET_SCRIPT_EVENT_SAVE_CONFIRMATION OFF EventSave eventArgVar
    SET_SCRIPT_EVENT_CHAR_CREATE OFF EventCharCreate eventArgVar
    SET_SCRIPT_EVENT_CHAR_DELETE OFF EventCharDelete eventArgVar
    SET_SCRIPT_EVENT_CAR_CREATE OFF EventCarCreate eventArgVar
    SET_SCRIPT_EVENT_CAR_DELETE OFF EventCarDelete eventArgVar
    SET_SCRIPT_EVENT_OBJECT_CREATE OFF EventObjectCreate eventArgVar
    SET_SCRIPT_EVENT_OBJECT_DELETE OFF EventObjectDelete eventArgVar
    SET_SCRIPT_EVENT_ON_MENU OFF EventOnMenu eventArgVar
    SET_SCRIPT_EVENT_CHAR_DAMAGE OFF EventCharDamage eventArgVar
    SET_SCRIPT_EVENT_CAR_WEAPON_DAMAGE OFF EventCarDamage eventArgVar
    SET_SCRIPT_EVENT_BULLET_IMPACT OFF EventBulletImpact i j k l
    PRINT_STRING_NOW "Events removed" 1000
    RETURN
    
    Test61:
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        i = 0
        WHILE GET_ANY_CHAR_NO_SAVE_RECURSIVE i (i j)
            GET_CHAR_COORDINATES j (x2 y2 z2)
            GET_CHAR_RANDOM_SEED j (k)
            GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED k 0 255 (m)
            k *= 5
            GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED k 0 255 (n)
            k *= 5
            GENERATE_RANDOM_INT_IN_RANGE_WITH_SEED k 0 255 (o)
            DRAW_SHADOW 3 x2 y2 z2 0.0 1.5 255 m n o
        ENDWHILE
    ENDWHILE
    RETURN

    Test60:
    GET_LABEL_POINTER Buffer128 i
    STRING_FORMAT i "test.dff"
    IF STRING_FIND STRING_FIND_LAST $i "." (j)
        IF IS_STRING_CHARACTER_AT $i "." j // this case will be always true, just testing
            CUT_STRING_AT i j
            SET_STRING_UPPER i
            PRINT_FORMATTED_NOW "%s" 1 i
        ENDIF
    ENDIF
    RETURN

    Test59:
    IF IS_FX_SYSTEM_AVAILABLE_WITH_NAME "prt_glass"
        GET_OFFSET_FROM_CAMERA_IN_WORLD_COORDS 0.0 5.0 0.0 (x y z)
        CREATE_FX_SYSTEM "prt_glass" (x y z) 1 i
        //GET_FX_SYSTEM_POINTER i j
        //PRINT_FORMATTED_NOW "%x" 1000 j
        REPEAT 10 j 
            ADD_FX_SYSTEM_PARTICLE i (x y z) (0.0 10.0 0.0) 0.2 1.0 (0.0 1.0 0.0 1.0) 1.0
        ENDREPEAT
    ENDIF
    RETURN
    
    Test58:
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        
        i = 0
        WHILE GET_ANY_CHAR_NO_SAVE_RECURSIVE i (i j)
            GET_CHAR_COORDINATES j (x2 y2 z2)
            DRAW_SHADOW 3 x2 y2 z2 0.0 1.5 255 255 0 0
        ENDWHILE

        i = 0
        WHILE GET_ANY_CAR_NO_SAVE_RECURSIVE i (i j)
            GET_CAR_COORDINATES j (x2 y2 z2)
            DRAW_SHADOW 3 x2 y2 z2 0.0 2.5 255 0 255 0
        ENDWHILE

        i = 0
        WHILE GET_ANY_OBJECT_NO_SAVE_RECURSIVE i (i j)
            GET_OBJECT_COORDINATES j (x2 y2 z2)
            DRAW_SHADOW 3 x2 y2 z2 0.0 1.5 255 0 0 255
        ENDWHILE
    ENDWHILE
    RETURN
    
    Test57:
    IF IS_MODEL_AVAILABLE_BY_NAME SWEET
        IF GET_MODEL_DOESNT_EXIST_IN_RANGE 15000 15024 i
            GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 5.0 0.0 x y z
            LOAD_SPECIAL_CHARACTER_FOR_ID i SWEET
            LOAD_ALL_MODELS_NOW
            CREATE_CHAR PEDTYPE_CIVMALE i x y z j
            PRINT_FORMATTED_NOW "Created with ID %i" 500 i
            WAIT 500
            DELETE_CHAR j
            UNLOAD_SPECIAL_CHARACTER_FROM_ID i
            
        ELSE
            PRINT_STRING_NOW "No model ID available to load special character" 2000
        ENDIF
    ELSE
        PRINT_STRING_NOW "There is no model with this name" 2000
    ENDIF
    RETURN
    
    Test56:
    IF IS_CHAR_REALLY_IN_AIR scplayer
        PRINT_STRING_NOW "on air %i" 1
    ENDIF
    RETURN

    Test55:
    GET_CURRENT_CHAR_WEAPONINFO scplayer i
    IF i > 0
        GET_WEAPONINFO_TOTAL_CLIP i j
        GET_CHAR_WEAPON_CLIP scplayer k
        PRINT_FORMATTED_NOW "Ammo: %i/%i" 1000 k j
    ENDIF
    RETURN

    Test54:
    GET_CURRENT_CHAR_WEAPONINFO scplayer i
    //GET_WEAPONINFO_MODELS i (j k)
    //PRINT_FORMATTED_NOW "WeaponInfo %x models %i %i" 1000 i j k
    //GET_WEAPONINFO_ANIMGROUP i (j)
    //PRINT_FORMATTED_NOW "WeaponInfo %x anim group %i" 1000 i j
    GET_WEAPONINFO_TOTAL_CLIP i (j)
    //PRINT_FORMATTED_NOW "WeaponInfo %x clip %i" 1000 i j
    /*GET_WEAPONINFO_FLAGS i (j)
    IF IS_LOCAL_VAR_BIT_SET_CONST j WEAPONINFO_FLAGS_TWINPISTOL
        PRINT_FORMATTED_NOW "WeaponInfo flags %x is twin pistol" 1000 j
    ELSE
        PRINT_FORMATTED_NOW "WeaponInfo flags %x is NOT twin pistol" 1000 j
    ENDIF*/
    RETURN

    Test53:
    IF IS_CHAR_SITTING_IN_ANY_CAR scplayer
        STORE_CAR_CHAR_IS_IN_NO_SAVE scplayer i
        IF DOES_CAR_HAVE_PART_NODE i CAR_NODE_EXHAUST // note: the game crashes if you try to install exhaust upgrade without exhaust node, so it's useful
            PRINT_STRING_NOW "Node: Yes" 1000
        ELSE
            PRINT_STRING_NOW "Node: Nope" 1000
        ENDIF
    ENDIF
    RETURN

    Test52:
    IF IS_STRING_COMMENT " #TESTE"
        PRINT_STRING_NOW "Is comment" 1000
    ELSE
        PRINT_STRING_NOW "Isn't comment" 1000
    ENDIF
    RETURN
    
    Test51:
    IF IS_STRING_EQUAL "TESTE" "T*sTE" 5 FALSE "*"
        PRINT_STRING_NOW "Equal" 1000
    ELSE
        PRINT_STRING_NOW "Different" 1000
    ENDIF
    RETURN

    Test50:
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE  
        WAIT 0
        DRAW_STRING "Test 1" DRAW_EVENT_BEFORE_HUD 300.0 300.0 0.25 0.5 1 FONT_SUBTITLES 
        DRAW_STRING_EXT "Test Extended" DRAW_EVENT_BEFORE_HUD (300.0 310.0 0.25 0.5) 1 FONT_SUBTITLES 1 ALIGN_LEFT 640.0 0 (255 255 255 255) 1 0 (0 0 0 255) 0 (0 0 0 0) 
    ENDWHILE
    RETURN

    Test49:
    // Create
    CREATE_LIST DATATYPE_STRING l
    // Populate
    LIST_ADD_STRING l "TEST1"
    LIST_ADD_STRING l "TEST2"
    LIST_ADD_STRING l "TEST3"
    LIST_ADD_STRING l "TEST4"
    LIST_ADD_STRING l "TEST5"
    // Iterate
    GET_LIST_SIZE l k
    i = 0
    WHILE i < k
        GET_LIST_STRING_VALUE_BY_INDEX l i (string)
        PRINT_FORMATTED_NOW "List value: %s" 1000 $string
        WAIT 500
        ++i
    ENDWHILE
    // Remove items
    LIST_REMOVE_INDEX_RANGE l 1 3
    // Iterate again
    GET_LIST_SIZE l k
    i = 0
    WHILE i < k
        GET_LIST_STRING_VALUE_BY_INDEX l i (string)
        PRINT_FORMATTED_NOW "List value: %s" 1000 $string
        WAIT 500
        ++i
    ENDWHILE
    // Delete
    DELETE_LIST l
    PRINT_STRING_NOW "List deleted" 1000
    RETURN

    Test48:
    // Create
    CREATE_LIST DATATYPE_INT l
    // Populate
    REPEAT 10 i
        LIST_ADD l i
    ENDREPEAT
    // Iterate
    GET_LIST_SIZE l k
    i = 0
    WHILE i < k
        GET_LIST_VALUE_BY_INDEX l i (j)
        PRINT_FORMATTED_NOW "List value: %i" 200 j
        WAIT 200
        ++i
    ENDWHILE
    // Clear
    RESET_LIST l
    // Iterate again (but there is nothing now)
    GET_LIST_SIZE l k
    i = 0
    WHILE i < k
        GET_LIST_VALUE_BY_INDEX l i (j)
        PRINT_FORMATTED_NOW "List value: %i" 200 j
        WAIT 200
        ++i
    ENDWHILE
    // Delete
    DELETE_LIST l
    PRINT_STRING_NOW "List deleted" 1000
    RETURN

    Test47:
    WHILE TRUE
        PRINT_FORMATTED_NOW "Running label %i" 1 i
        IF TEST_CHEAT "EXIT"
            TERMINATE_THIS_CUSTOM_SCRIPT
        ENDIF
        WAIT 0
    ENDWHILE
    RETURN

    Test46:
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 0.0 0.0 (x y z)
        GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 0.0 -5.0 (x2 y2 z2)
        GET_PED_POINTER scplayer i
        GET_LABEL_POINTER Buffer128 j // at least 44 bytes is required. If you don't send a pointer, it will work anyway, but COLPOINT will not return
        IF GET_COLLISION_BETWEEN_POINTS (x y z) (x2 y2 z2) TRUE TRUE TRUE TRUE TRUE TRUE TRUE TRUE i j (x y z i)
            GET_COLPOINT_DEPTH j (g)
            GET_COLPOINT_SURFACE j (l)
            GET_COLPOINT_LIGHTING j FALSE (k)
            GET_COLPOINT_NORMAL_VECTOR j x2 y2 z2
            PRINT_FORMATTED_NOW "Col %.2f %.2f %.2f entity %x~n~depth %.2f surf %i light %i norm %.2f %.2f %.2f" 1000 x y z i g l k x2 y2 z2
        ENDIF
    ENDWHILE
    RETURN

    Test45:
    MAKE_NOP 0x53BFBD 5 // NOPs the clock update, vp is true
    RETURN

    Test44:
    i = 3000 // delay
    PRINT_STRING_NOW "Pad control off (doesn't make player safe)" i
    SET_PLAYER_CONTROL_PAD PAD1 OFF
    WAIT i
    PRINT_STRING_NOW "Pad control movement off (camera and commands still work)" i
    SET_PLAYER_CONTROL_PAD PAD1 ON
    SET_PLAYER_CONTROL_PAD_MOVEMENT PAD1 OFF
    WAIT i
    PRINT_STRING_NOW "Pad control on, camera control off" i
    SET_PLAYER_CONTROL_PAD_MOVEMENT PAD1 ON
    SET_CAMERA_CONTROL OFF
    WAIT i
    PRINT_STRING_NOW "Camera control on" i
    SET_CAMERA_CONTROL ON
    RETURN

    Test43:
    IF IS_CHAR_SITTING_IN_ANY_CAR scplayer
        STORE_CAR_CHAR_IS_IN_NO_SAVE scplayer i
        WHILE DOES_VEHICLE_EXIST i
        AND NOT IS_KEY_PRESSED VK_KEY_N
            IF GET_CAR_COLLISION_INTENSITY i g // CAUTION, this may return some wierd large values on Rhino, and maybe helicopters
                GET_CAR_COLLISION_COORDINATES i x y z
                PRINT_FORMATTED_NOW "Last collision intensity is %.3f pos %.3f %.3f %.3f. Press N to exit" 1000 g x y z
            ENDIF
            WAIT 0
        ENDWHILE
    ENDIF
    RETURN

    Test42:
    GET_CHAR_MAX_HEALTH scplayer g
    GET_CHAR_HEALTH_PERCENT scplayer g1
    PRINT_FORMATTED_NOW "maxhealth: %f percent: %f" 1000 g g1
    RETURN

    Test41:
    GET_ACTIVE_CAMERA_COORDINATES x y z
    IF GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE x y z 10.0 FALSE FALSE i
        SET_CAR_ALARM i CAR_ALARM_PLAYING
        PRINT_STRING_NOW "Car alarm set" 1000
    ELSE
        PRINT_STRING_NOW "Car not found" 1000
    ENDIF
    RETURN
    
    Test40:
    PRINT_STRING_NOW "Cam control off" 2000
    SET_CAMERA_CONTROL 0
    WAIT 2000
    PRINT_STRING_NOW "Cam control on" 1000
    SET_CAMERA_CONTROL 1
    RETURN

    Test39:
    WHILE TRUE
        WAIT 0
        IF IS_KEY_PRESSED VK_KEY_Y
            g +=@ 0.1
        ENDIF
        IF IS_KEY_PRESSED VK_KEY_N
            g -=@ 0.1
        ENDIF

        PERLIN_NOISE_FRACTAL g 2 0.5 1.0 1.0 1.0 (g1)
        PRINT_FORMATTED_NOW "Gradient noise: %f" 1 g1

        g1 *= 100.0

        x = 300.0
        y = 300.0 + g1
        GET_FIXED_XY_ASPECT_RATIO 50.0 50.0 x2 y2
        DRAW_RECT x y x2 y2 255 255 255 255
        USE_TEXT_COMMANDS 0
    ENDWHILE
    RETURN

    Test38:
    GET_LABEL_POINTER Buffer128 i
    STRING_FORMAT i "Testing"
    GET_STRING_LENGTH $i j
    PRINT_FORMATTED_NOW "'%s' len = %i" 1000 i j
    RETURN

    Test37:
    IF IS_CHAR_SITTING_IN_ANY_CAR scplayer
        STORE_CAR_CHAR_IS_IN_NO_SAVE scplayer i
        WHILE DOES_VEHICLE_EXIST i
        AND NOT IS_KEY_PRESSED VK_KEY_N
            PRINT_STRING_NOW "Finally, now you can horn by script... Press Y to horn, N to exit" 1
            IF IS_KEY_PRESSED VK_KEY_Y
                CAR_HORN i
            ENDIF
            WAIT 0
        ENDWHILE
    ENDIF
    RETURN

    Test36:
    CHANGE_PLAYER_MONEY 0 CHANGE_MONEY_SET 1000000 // updates instantly
    CHANGE_PLAYER_MONEY 0 CHANGE_MONEY_ADD 30
    CHANGE_PLAYER_MONEY 0 CHANGE_MONEY_REMOVE 10
    // = 1000020
    RETURN

    Test35:
    WHILE NOT IS_BUTTON_PRESSED PAD1 TRIANGLE
        WAIT 0
        IF IS_CHEAT_ACTIVE 70
            PRINT_STRING_NOW "CrazyTown is active" 1
        ELSE
            PRINT_STRING_NOW "CrazyTown is NOT active" 1
        ENDIF
    ENDWHILE
    RETURN

    Test34:
    GET_VAR_POINTER i j
    GET_VAR_POINTER k l
    k = 10
    i = 20
    COPY_MEMORY j l 4
    PRINT_FORMATTED_NOW "i=%i" 5000 i // outputs '10'
    RETURN

    Test33:
    IF IS_CHAR_SITTING_IN_ANY_CAR scplayer
        STORE_CAR_CHAR_IS_IN_NO_SAVE scplayer i
        WHILE DOES_VEHICLE_EXIST i
            IF GET_CAR_DUMMY_COORD i VEHICLE_DUMMY_EXHAUST TRUE FALSE x y z
                //GET_OFFSET_FROM_CAR_IN_WORLD_COORDS i (x y z) (x y z)
                DRAW_WEAPONSHOP_CORONA x y z 1.0 CORONATYPE_SHINYSTAR FLARETYPE_NONE 255 255 255
                PRINT_FORMATTED_NOW "%.3f %.3f %.3f" 100 x y z
            ENDIF
            WAIT 0
        ENDWHILE
    ENDIF
    RETURN

    Test32:
    IF IS_CHAR_SITTING_IN_ANY_CAR scplayer
        STORE_CAR_CHAR_IS_IN_NO_SAVE scplayer i
        IF GET_TRAILER_FROM_CAR i j
            EXPLODE_CAR j
            IF GET_CAR_FROM_TRAILER j k
                EXPLODE_CAR k
            ENDIF
        ENDIF
    ENDIF
    RETURN

    Test31:
    //DISPLAY_ONSCREEN_TIMER_LOCAL i TIMER_DOWN
    DISPLAY_ONSCREEN_TIMER_WITH_STRING_LOCAL k TIMER_DOWN "BB_19"
    //DISPLAY_ONSCREEN_COUNTER_LOCAL i COUNTER_DISPLAY_NUMBER
    //DISPLAY_ONSCREEN_COUNTER_WITH_STRING_LOCAL k 0 BB_19
    DISPLAY_TWO_ONSCREEN_COUNTERS_WITH_STRING_LOCAL i j BB_19
     
    SET_ONSCREEN_COUNTER_FLASH_WHEN_FIRST_DISPLAYED_LOCAL i FALSE
    SET_ONSCREEN_COUNTER_COLOUR_LOCAL i 1
    SET_TIMER_BEEP_COUNTDOWN_TIME_LOCAL k 3
    k = 5000
    i = 50
    j = 100

    WAIT 3000

    CLEAR_ONSCREEN_TIMER_LOCAL k
    CLEAR_ONSCREEN_COUNTER_LOCAL i
    RETURN

    Test30:
    IF FRAME_MOD 2 // it will return true each 2 frames, very used for optimization
        PRINT_STRING_NOW "1" 10
    ELSE
        PRINT_STRING_NOW "2" 10
    ENDIF
    RETURN

    Test29:
    IF LOAD_AUDIO_STREAM "CLEO/Test/test.mp3" i
        SET_AUDIO_STREAM_STATE i 1
        WAIT 3000 // it will play for 3 segs before continue
        GET_AUDIOSTREAM_INTERNAL i j
        // now you can use bass functions www.un4seen.com/doc/ (HSTREAM)
        PRINT_FORMATTED_NOW "audiostream internal %x" 2000 j
        IF LOAD_DYNAMIC_LIBRARY "bass.dll" i
            IF GET_DYNAMIC_LIBRARY_PROCEDURE "BASS_ChannelStop" i k //BOOL BASS_ChannelStop(DWORD handle);
                CALL_FUNCTION k 1 0 (j)
            ENDIF
            FREE_DYNAMIC_LIBRARY i
        ENDIF
    ENDIF
    RETURN

    Test28:
    LOAD_TEXTURE_DICTIONARY afro
    LOAD_SPRITE 1 afrobeard
    GET_TEXTURE_FROM_SPRITE 1 i
    WRITE_MEMORY 0xB4E690 4 i FALSE // CVehicleModelInfo::ms_pLightsOnTexture (this test doesn't work with IVF cars)
    PRINT_FORMATTED_NOW "sprite texture %x" 1000 i
    // using loaded sprite for blips:
    ADD_CLEO_BLIP -1 10.0 10.0 TRUE 255 255 255 255 j // by loaded txd sprite ID, just use the sprite index but negative
    ADD_CLEO_BLIP i 90.0 90.0 TRUE 255 255 255 255 j // by texture pointer, it understand it as pointer
    RETURN

    Test27:
    STORE_CLOSEST_ENTITIES scplayer car char
    IF GET_CHAR_KILL_TARGET_CHAR char i
        PRINT_FORMATTED_NOW "target %x" 1000 i
    ENDIF
    RETURN

    Test26:
    IF GET_CHAR_TASK_POINTER_BY_ID scplayer TASK_SIMPLE_USE_GUN i
        PRINT_FORMATTED_NOW "task gun %x" 1000 i
    ENDIF
    RETURN
    
    Test25:
    GET_CURRENT_HOUR i // because you always need to provide minute variable to GET_TIME_OF_DAY, but you normally never need it
    GET_CURRENT_MINUTE j
    PRINT_FORMATTED_NOW "%i:%i" 1000 i j
    RETURN

    Test24:
    ADD_TEXT_LABEL CLEOP01 "Busy"
    WHILE TRUE
        WAIT 0

        j = 0
        WHILE GET_ANY_CHAR_NO_SAVE_RECURSIVE j (j i)
            GOSUB Test24_Sub
        ENDWHILE

        IF TEST_CHEAT EXIT
            BREAK
        ENDIF
    ENDWHILE
    RETURN
    
    Test24_Sub:
    IF IS_CHAR_DOING_ANY_IMPORTANT_TASK i INCLUDE_ANIMS_BOTH
        GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS i 0.0 0.5 0.0 g g1 g2
        IF CONVERT_3D_TO_SCREEN_2D g g1 g2 TRUE TRUE x y x2 y2
            SET_TEXT_SCALE x2 y2
            DISPLAY_TEXT x y CLEOP01
            USE_TEXT_COMMANDS 0
        ENDIF
    ENDIF
    RETURN

    Test23:
    k = 1000
    x = 0.0
    y = 0.0
    z = 0.0
    x2 = 0.0
    y2 = 0.0
    z2 = 0.0
    g = 1.0
    g1 = 1.0
    g2 = 1.0
    i = 0
    j = 0
    l = TRUE
    PRINT_STRING_NOW "Hold - + and use B M P R S D H L XYZ for model, rotation etc" 2000
    WAIT 2000
    GOSUB Test23_RecreateRenderObject
    SET_PLAYER_CONTROL_PAD_MOVEMENT 0 l
    WHILE TRUE
        WAIT 0
        IF IS_KEY_PRESSED 107
            IF IS_KEY_PRESSED VK_LSHIFT
                IF IS_KEY_JUST_PRESSED VK_KEY_B
                    j += 10
                ENDIF
                IF IS_KEY_JUST_PRESSED VK_KEY_M
                    k += 10
                ENDIF
                IF IS_KEY_PRESSED VK_KEY_R
                    IF IS_KEY_PRESSED VK_KEY_X
                        x2 +=@ 10.0
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Y
                        y2 +=@ 10.0
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Z
                        z2 +=@ 10.0
                    ENDIF
                ENDIF
                IF IS_KEY_PRESSED VK_KEY_S
                    IF IS_KEY_PRESSED VK_KEY_X
                        g +=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Y
                        g1 +=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Z
                        g2 +=@ 0.01
                    ENDIF
                ENDIF
                IF IS_KEY_PRESSED VK_KEY_P
                    IF IS_KEY_PRESSED VK_KEY_X
                        x +=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Y
                        y +=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Z
                        z +=@ 0.01
                    ENDIF
                ENDIF
            ELSE
                IF IS_KEY_PRESSED VK_LCONTROL
                    IF IS_KEY_JUST_PRESSED VK_KEY_B
                        j += 100
                    ENDIF
                    IF IS_KEY_JUST_PRESSED VK_KEY_M
                        k += 100
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_R
                        IF IS_KEY_PRESSED VK_KEY_X
                            x2 +=@ 20.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y2 +=@ 20.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z2 +=@ 20.0
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_S
                        IF IS_KEY_PRESSED VK_KEY_X
                            g +=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            g1 +=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            g2 +=@ 0.1
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_P
                        IF IS_KEY_PRESSED VK_KEY_X
                            x +=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y +=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z +=@ 0.1
                        ENDIF
                    ENDIF
                ELSE
                    IF IS_KEY_JUST_PRESSED VK_KEY_B
                        j += 1
                    ENDIF
                    IF IS_KEY_JUST_PRESSED VK_KEY_M
                        k += 1
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_R
                        IF IS_KEY_PRESSED VK_KEY_X
                            x2 +=@ 1.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y2 +=@ 1.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z2 +=@ 1.0
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_S
                        IF IS_KEY_PRESSED VK_KEY_X
                            g +=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            g1 +=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            g2 +=@ 0.001
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_P
                        IF IS_KEY_PRESSED VK_KEY_X
                            x +=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y +=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z +=@ 0.001
                        ENDIF
                    ENDIF
                ENDIF
            ENDIF
            GOSUB Test23_RecreateRenderObject
        ENDIF
        IF IS_KEY_PRESSED 109
            IF IS_KEY_PRESSED VK_LSHIFT
                IF IS_KEY_JUST_PRESSED VK_KEY_B
                    j -= 10
                ENDIF
                IF IS_KEY_JUST_PRESSED VK_KEY_M
                    k -= 10
                ENDIF
                IF IS_KEY_PRESSED VK_KEY_R
                    IF IS_KEY_PRESSED VK_KEY_X
                        x2 -=@ 10.0
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Y
                        y2 -=@ 10.0
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Z
                        z2 -=@ 10.0
                    ENDIF
                ENDIF
                IF IS_KEY_PRESSED VK_KEY_S
                    IF IS_KEY_PRESSED VK_KEY_X
                        g -=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Y
                        g1 -=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Z
                        g2 -=@ 0.01
                    ENDIF
                ENDIF
                IF IS_KEY_PRESSED VK_KEY_P
                    IF IS_KEY_PRESSED VK_KEY_X
                        x -=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Y
                        y -=@ 0.01
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_Z
                        z -=@ 0.01
                    ENDIF
                ENDIF
            ELSE
                IF IS_KEY_PRESSED VK_LCONTROL
                    IF IS_KEY_JUST_PRESSED VK_KEY_B
                        j -= 100
                    ENDIF
                    IF IS_KEY_JUST_PRESSED VK_KEY_M
                        k -= 100
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_R
                        IF IS_KEY_PRESSED VK_KEY_X
                            x2 -=@ 20.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y2 -=@ 20.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z2 -=@ 20.0
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_S
                        IF IS_KEY_PRESSED VK_KEY_X
                            g -=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            g1 -=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            g2 -=@ 0.1
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_P
                        IF IS_KEY_PRESSED VK_KEY_X
                            x -=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y -=@ 0.1
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z -=@ 0.1
                        ENDIF
                    ENDIF
                ELSE
                    IF IS_KEY_JUST_PRESSED VK_KEY_B
                        j -= 1
                    ENDIF
                    IF IS_KEY_JUST_PRESSED VK_KEY_M
                        k -= 1
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_R
                        IF IS_KEY_PRESSED VK_KEY_X
                            x2 -=@ 1.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y2 -=@ 1.0
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z2 -=@ 1.0
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_S
                        IF IS_KEY_PRESSED VK_KEY_X
                            g -=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            g1 -=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            g2 -=@ 0.001
                        ENDIF
                    ENDIF
                    IF IS_KEY_PRESSED VK_KEY_P
                        IF IS_KEY_PRESSED VK_KEY_X
                            x -=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Y
                            y -=@ 0.001
                        ENDIF
                        IF IS_KEY_PRESSED VK_KEY_Z
                            z -=@ 0.001
                        ENDIF
                    ENDIF
                ENDIF
            ENDIF
            GOSUB Test23_RecreateRenderObject
        ENDIF
        IF IS_KEY_JUST_PRESSED VK_KEY_L
            IF l = TRUE
                l = FALSE
                PRINT_STRING_NOW "Free!" 1000
            ELSE
                l = TRUE
                PRINT_STRING_NOW "Lock!" 1000
            ENDIF
            SET_PLAYER_CONTROL_PAD_MOVEMENT 0 l
        ENDIF
        IF IS_KEY_JUST_PRESSED VK_KEY_H
            i = 0 // don't delete it
            PRINT_STRING_NOW "Saved!" 1000
            GOSUB Test23_RecreateRenderObject
        ENDIF
        IF IS_KEY_PRESSED VK_KEY_N // exit
            PRINT_STRING_NOW "~r~Hold to exit" 2000
            WAIT 2000
            IF IS_KEY_PRESSED VK_KEY_N
                BREAK
            ENDIF
        ENDIF
        PRINT_FORMATTED_NOW "model %i bone id %i" 1000 k j
    ENDWHILE
    //CREATE_RENDER_OBJECT_TO_CHAR_BONE scplayer 1001 5 (-0.5 0.0 0.0) (0.0 0.0 0.0) i
    //SET_RENDER_OBJECT_AUTO_HIDE i TRUE TRUE TRUE
    //SET_RENDER_OBJECT_VISIBLE i FALSE
    SET_PLAYER_CONTROL_PAD_MOVEMENT 0 ON
    RETURN

    Test23_RecreateRenderObject:
    IF IS_MODEL_IN_CDIMAGE k
        REQUEST_MODEL k
        LOAD_ALL_MODELS_NOW
        DELETE_RENDER_OBJECT i
        CREATE_RENDER_OBJECT_TO_CHAR_BONE scplayer k j (x y z) (x2 y2 z2) i
        SET_RENDER_OBJECT_SCALE i g g1 g2
        MARK_MODEL_AS_NO_LONGER_NEEDED k
    ENDIF
    RETURN

    Test22:
    CREATE_PICKUP 1277 5 10.0 10.0 5.0 j
    IF GET_PICKUP_THIS_COORD 10.0 10.0 5.0 TRUE i
        //GET_PICKUP_COORDINATES i x y z
        //PRINT_FORMATTED_NOW "%.3f %.3f %.3f" 1000 x y z
        GET_PICKUP_MODEL i j
        GET_PICKUP_TYPE i k
        //PRINT_FORMATTED_NOW "%.3f %.3f %.3f" 1000 x y z
        PRINT_FORMATTED_NOW "pickup model %i type %i" 1000 j k
    ENDIF
    RETURN 

    Test21:
    REQUEST_MODEL 1000
    REQUEST_MODEL 1001
    LOAD_ALL_MODELS_NOW
    CREATE_RENDER_OBJECT_TO_CHAR_BONE scplayer 1000 5 (0.0 0.0 0.0) (0.0 0.0 0.0) i
    CREATE_RENDER_OBJECT_TO_CHAR_BONE scplayer 1001 5 (-0.5 0.0 0.0) (0.0 0.0 0.0) i
    SET_RENDER_OBJECT_AUTO_HIDE i TRUE TRUE TRUE
    //SET_RENDER_OBJECT_VISIBLE i FALSE
    RETURN

    Test20:
    GET_CURRENT_SAVE_SLOT i
    IF IS_GAME_FIRST_START
        PRINT_FORMATTED_NOW "Current save slot %d. First start." 2000 i
    ELSE
        PRINT_FORMATTED_NOW "Current save slot %d. Not first start." 2000 i
    ENDIF
    RETURN

    Test19:
    ADD_CLEO_BLIP 35 100.0 100.0 FALSE 255 255 255 255 (i)
    WAIT 1000
    //REMOVE_CLEO_BLIP i
    RETURN

    Test18:
    GET_CURRENT_CHAR_WEAPON scplayer i
    IF IS_WEAPON_FIRE_TYPE i WEAPONFIRE_INSTANT_HIT
        PRINT_STRING_NOW "is firegun" 1000
    ENDIF
    RETURN
    
    Test17:
    GET_PED_POINTER scplayer i
    READ_STRUCT_OFFSET i 0x540 4 (g) //00000540 m_fHealth dd
    PRINT_FORMATTED_NOW "%f" 1000 g
    RETURN

    Test16:
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 2.0 0.0 x y z
    CREATE_CHAR PEDTYPE_CIVMALE MALE01 x y z (char)
    FIX_CHAR_GROUND_BRIGHTNESS_AND_FADE_IN char TRUE TRUE TRUE
    RETURN

    Test15:
    GET_AUDIO_SFX_VOLUME g1
    PRINT_FORMATTED_NOW "sfx %.3f" 1000 g1
    WAIT 1000
    GET_AUDIO_RADIO_VOLUME g1
    PRINT_FORMATTED_NOW "radio %.3f" 1000 g1
    WAIT 1000
    GET_MOUSE_SENSIBILITY g1
    PRINT_FORMATTED_NOW "mouse sens %.3f" 1000 g1
    RETURN

    Test14:
    IF IS_ON_MISSION
        PRINT_STRING_NOW "ON MISSION" 100
    ELSE
        PRINT_STRING_NOW "OFF MISSION" 100
    ENDIF
    RETURN

    Test13:
    g = 0.0
    WHILE g < 1.0
        WAIT 0
        g +=@ 0.01 // linear interpolation
        EASE g EASE_MODE_CUBIC EASE_WAY_INOUT (g2) // get ease interpolation from linear
        PRINT_FORMATTED_NOW "%.3f ease %.3f" 10 g g2
        g1 = 100.0 * g2 // use ease for distance to travel progress
        g1 += 200.0 // add original position
        DRAW_RECT 200.0 g1 20.0 20.0 255 255 255 255
        USE_TEXT_COMMANDS 0
    ENDWHILE
    RETURN

    /*
    Test12:
    STORE_CLOSEST_ENTITIES scplayer car char
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 5.0 0.0 (x y z)
    REQUEST_MODEL 400
    LOAD_ALL_MODELS_NOW
    CREATE_CAR 400 x y z (obj)
    SET_CAR_HEADING obj 120.0
    GET_VEHICLE_QUATERNION obj quat_x quat_y quat_z quat_w
    GET_VEHICLE_QUATERNION car dest_quat_x dest_quat_y dest_quat_z dest_quat_w
    GET_VAR_POINTER quat_x (source_quat)
    GET_VAR_POINTER dest_quat_x (dest_quat)
    GET_VAR_POINTER res_quat_x (res_quat)
    WHILE TRUE
        WAIT 0
        QUAT_SLERP source_quat dest_quat 0.5 (res_quat)
        SET_VEHICLE_QUATERNION obj res_quat_x res_quat_y res_quat_z res_quat_w
    ENDWHILE
    RETURN
    */

    Test11a:
    STORE_CLOSEST_ENTITIES scplayer car char
    IF car > 0
        INIT_EXTENDED_CHAR_VARS car AUTO 10
        SET_EXTENDED_CHAR_VAR car AUTO 1 100
        PRINT_FORMATTED_NOW "Set car %i" 2000 500
    ENDIF
    RETURN 
    
    Test11b:
    
    STORE_CLOSEST_ENTITIES scplayer car char
    IF car > 0
        IF GET_EXTENDED_CAR_VAR car AUTO 1 (i) // Get var 1 value
            PRINT_FORMATTED_NOW "Get var %i" 2000 i // In this example, i is 100
        ELSE
            PRINT_STRING "No value yet for var. Writing" 2000
            // Initialize 2 var spaces
            // Use AUTO for automatically use this script as id, you can use some number too
            INIT_EXTENDED_CAR_VARS car AUTO 2 
            SET_EXTENDED_CAR_VAR car AUTO 1 100 // Write 100 in var 1
            SET_EXTENDED_CAR_VAR car AUTO 2 200 // Write 200 in var 1
        ENDIF
    ENDIF

    RETURN

    Test10:
    LOAD_TEXTURE_DICTIONARY LD_CHAT
    LOAD_SPRITE 1 thumbup
    GET_TEXTURE_FROM_SPRITE 1 i
    GET_LABEL_POINTER Buffer128 k
    WRITE_STRUCT_OFFSET k 0  4 100.0  // x right 1
    WRITE_STRUCT_OFFSET k 4  4 100.0  // y top 1
    WRITE_STRUCT_OFFSET k 8  4 200.0  // x left 1
    WRITE_STRUCT_OFFSET k 12 4 100.0  // y bottom 1
    WRITE_STRUCT_OFFSET k 16 4 100.0  // x right 2
    WRITE_STRUCT_OFFSET k 20 4 200.0  // y top 2
    WRITE_STRUCT_OFFSET k 24 4 200.0  // x left 2
    WRITE_STRUCT_OFFSET k 28 4 250.0  // y bottom 2

    GET_LABEL_POINTER Buffer128B j
    WRITE_STRUCT_OFFSET j 0  4 300.0  // x right 1
    WRITE_STRUCT_OFFSET j 4  4 300.0  // y top 1
    WRITE_STRUCT_OFFSET j 8  4 350.0  // x left 1
    WRITE_STRUCT_OFFSET j 12 4 300.0  // y bottom 1
    WRITE_STRUCT_OFFSET j 16 4 300.0  // x right 2
    WRITE_STRUCT_OFFSET j 20 4 350.0  // y top 2
    WRITE_STRUCT_OFFSET j 24 4 350.0  // x left 2
    WRITE_STRUCT_OFFSET j 28 4 350.0  // y bottom 2

    x = 80.0
    y = 80.0

    WHILE TRUE
        WAIT 0

        IF IS_KEY_PRESSED 107
            g +=@ 5.0
        ENDIF
        IF IS_KEY_PRESSED 109
            g -=@ 5.0
        ENDIF
        
        IF IS_KEY_PRESSED VK_NUMPAD6
            x +=@ 5.0
        ENDIF
        IF IS_KEY_PRESSED VK_NUMPAD4
            x -=@ 5.0
        ENDIF
        IF IS_KEY_PRESSED VK_NUMPAD5
            y +=@ 5.0
        ENDIF
        IF IS_KEY_PRESSED VK_NUMPAD8
            y -=@ 5.0
        ENDIF

        DRAW_TEXTURE_PLUS i DRAW_EVENT_BEFORE_HUD (x y) (200.0 200.0) g 0.0 TRUE 4 k 255 255 255 180

        DRAW_TEXTURE_PLUS i DRAW_EVENT_BEFORE_HUD (300.0 300.0) (200.0 200.0) 0.0 0.0 TRUE 4 j 255 255 255 180
        
        PRINT_STRING_NOW "Drawning, (pos NUMPAD) (angle -+)" 10
    ENDWHILE
    RETURN
    
    Test9:
    GET_LABEL_POINTER Buffer128 pBuffer
    STRING_FORMAT pBuffer "TESTE%i" 50
    PRINT_FORMATTED_NOW "%s" 5000 $pBuffer
    WAIT 1000
    INIT_EXTENDED_CHAR_VARS scplayer $pBuffer 10
    SET_EXTENDED_CHAR_VAR scplayer $pBuffer 1 500
    //GET_EXTENDED_CHAR_VAR scplayer $pBuffer 1 (i)
    PRINT_FORMATTED_NOW "%i" 5000 i
    RETURN
    
    Test8a:
    STORE_CLOSEST_ENTITIES scplayer car char
    IF char > 0
        INIT_EXTENDED_CHAR_VARS char AUTO 10
        SET_EXTENDED_CHAR_VAR char AUTO 1 500
        PRINT_FORMATTED_NOW "Set %i" 2000 500
    ENDIF
    RETURN

    Test8b:
    STORE_CLOSEST_ENTITIES scplayer car char
    IF char > 0
        IF GET_EXTENDED_CHAR_VAR char AUTO 1 (i)
            PRINT_FORMATTED_NOW "Get %i" 2000 i
        ELSE
            PRINT_STRING "No value yet" 2000
        ENDIF
    ENDIF
    RETURN

    Test8:
    INIT_EXTENDED_CHAR_VARS scplayer AUTO 10
    SET_EXTENDED_CHAR_VAR scplayer AUTO 2 400
    GET_EXTENDED_CHAR_VAR scplayer AUTO 2 (i)
    PRINT_FORMATTED_NOW "%i" 5000 i
    RETURN
  
    Test7:
    IF IS_CHAR_SITTING_IN_ANY_CAR scplayer

        STORE_CAR_CHAR_IS_IN_NO_SAVE scplayer car

        GET_VEHICLE_SUBCLASS car i

        IF i = VEHICLE_SUBCLASS_BIKE
        OR i = VEHICLE_SUBCLASS_TRAIN
            IF IS_CAR_SCRIPT_CONTROLLED car
                PRINT_FORMATTED_NOW "%i is script motorbike or train" 2000 i
            ELSE
                PRINT_FORMATTED_NOW "%i is random motorbike or train" 2000 i
            ENDIF
        ENDIF

        WAIT 1000
        GET_VEHICLE_POINTER car i
        GET_ENTITY_TYPE i i
        PRINT_FORMATTED_NOW "entity type %i" 2000 i
    ENDIF
    RETURN
    
    Test6:
    timera = 0
    WHILE timera < 5000
        WAIT 0
        IF IS_MOUSE_WHEEL_UP
            PRINT_STRING_NOW "wheel up" 1
        ENDIF
        IF IS_MOUSE_WHEEL_DOWN
            PRINT_STRING_NOW "wheel down" 1
        ENDIF
    ENDWHILE
    RETURN

    Test5:
    GET_CURRENT_RESOLUTION j k
    PRINT_FORMATTED_NOW "%i %i" 5000 j k

    WAIT 1000

    timera = 0
    WHILE timera < 5000
        WAIT 0
        GET_FIXED_XY_ASPECT_RATIO 50.0 50.0 x y
        DRAW_RECT 320.0 224.0 x y 255 255 255 255
        USE_TEXT_COMMANDS 0
    ENDWHILE
    RETURN

    Test4:
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 2.0 0.0 (x y z)
    CREATE_OBJECT 1000 x y z (obj)

    IF IS_OBJECT_SCRIPT_CONTROLLED obj
        PRINT_STRING_NOW "obj created TRUE" 1000
    ELSE
        PRINT_STRING_NOW "~r~obj created FALSE: error" 1000
    ENDIF

    WAIT 1000

    MARK_OBJECT_AS_NO_LONGER_NEEDED obj

    IF IS_OBJECT_SCRIPT_CONTROLLED obj
        PRINT_STRING_NOW "~r~obj released TRUE: error" 1000
    ELSE
        PRINT_STRING_NOW "obj released FALSE" 1000
    ENDIF
    
    WAIT 1000
    MARK_OBJECT_AS_NEEDED obj 

    IF IS_OBJECT_SCRIPT_CONTROLLED obj
        PRINT_STRING_NOW "obj marked TRUE" 1000
    ELSE
        PRINT_STRING_NOW "~r~obj marked FALSE: error" 1000
    ENDIF
    RETURN

    Test3:
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 2.0 0.0 (x y z)
    CREATE_CHAR PEDTYPE_CIVMALE MALE01 x y z (char)

    IF IS_CHAR_SCRIPT_CONTROLLED char
        PRINT_STRING_NOW "char created TRUE" 1000
    ELSE
        PRINT_STRING_NOW "~r~char created FALSE: error" 1000
    ENDIF

    WAIT 1000

    MARK_CHAR_AS_NO_LONGER_NEEDED char

    IF IS_CHAR_SCRIPT_CONTROLLED char
        PRINT_STRING_NOW "~r~char released TRUE: error" 1000
    ELSE
        PRINT_STRING_NOW "char released FALSE" 1000
    ENDIF
    
    WAIT 1000
    MARK_CHAR_AS_NEEDED char 

    IF IS_CHAR_SCRIPT_CONTROLLED char
        PRINT_STRING_NOW "char marked TRUE" 1000
    ELSE
        PRINT_STRING_NOW "~r~char marked FALSE: error" 1000
    ENDIF
    RETURN

    Test2:
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 2.0 0.0 (x y z)
    CREATE_CAR ELEGY x y z (car)

    IF IS_CAR_SCRIPT_CONTROLLED car
        PRINT_STRING_NOW "car created TRUE" 1000
    ELSE
        PRINT_STRING_NOW "~r~car created FALSE: error" 1000
    ENDIF

    WAIT 1000

    MARK_CAR_AS_NO_LONGER_NEEDED car

    IF IS_CAR_SCRIPT_CONTROLLED car
        PRINT_STRING_NOW "~r~car released TRUE: error" 1000
    ELSE
        PRINT_STRING_NOW "car released FALSE" 1000
    ENDIF

    WAIT 1000

    MARK_CAR_AS_NEEDED car 

    IF IS_CAR_SCRIPT_CONTROLLED car
        PRINT_STRING_NOW "car marked TRUE" 1000
    ELSE
        PRINT_STRING_NOW "~r~car marked FALSE: error" 1000
    ENDIF
    RETURN

    Test1:
    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 0.0 2.0 0.0 (x y z)
    CREATE_OBJECT_NO_SAVE 1000 x y z FALSE FALSE (obj)

    GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS scplayer 20.0 5.0 0.0 (x y z)
    CREATE_CAR_GENERATOR x y z 0.0 ELEGY 1 1 TRUE 0 0 1000 2000 car
    SWITCH_CAR_GENERATOR car 101
    SET_CAR_GENERATOR_NO_SAVE car

    GET_CURRENT_WEATHER i
    PRINT_FORMATTED_NOW "cur weather id %i" 1000 i
    
    WAIT 1000

    SET_NEXT_WEATHER 8 

    GET_NEXT_WEATHER i
    PRINT_FORMATTED_NOW "next weather id %i" 1000 i

    WAIT 1000

    SET_RAIN_INTENSITY 0.5

    GET_RAIN_INTENSITY g
    PRINT_FORMATTED_NOW "rain %f" 1000 g

    RETURN


    EventSave:
    PRINT_FORMATTED_NOW "Save: %i" 2000 eventArgVar
    RETURN_SCRIPT_EVENT
    
    EventCharCreate:
    PRINT_FORMATTED "Char create: %i" 100 eventArgVar
    RETURN_SCRIPT_EVENT

    EventCharDelete:
    PRINT_FORMATTED "Char delete: %i" 100 eventArgVar
    RETURN_SCRIPT_EVENT

    EventCarCreate:
    PRINT_FORMATTED "Car create: %i" 100 eventArgVar
    RETURN_SCRIPT_EVENT
    
    EventCarDelete:
    PRINT_FORMATTED "Car delete: %i" 100 eventArgVar
    RETURN_SCRIPT_EVENT

    EventObjectCreate:
    PRINT_FORMATTED "Obj create: %i" 100 eventArgVar
    RETURN_SCRIPT_EVENT
    
    EventObjectDelete:
    PRINT_FORMATTED "Obj delete: %i" 100 eventArgVar
    RETURN_SCRIPT_EVENT

    EventCharDamage:
    // It's better to use event this case because it can return multiple times each frame, so, you will not loose any damage
    GET_CHAR_DAMAGE_LAST_FRAME eventArgVar i j k g
    PRINT_FORMATTED_NOW "Char damage: %i %i %i %i %.3f" 1000 eventArgVar i j k g
    RETURN_SCRIPT_EVENT
    
    EventCarDamage:
    GET_CAR_WEAPON_DAMAGE_LAST_FRAME eventArgVar i j g
    PRINT_FORMATTED_NOW "Car damage: %i %i %i %.3f" 1000 eventArgVar i j g
    RETURN_SCRIPT_EVENT
    
    EventBulletImpact:
    // i = entity owner, j = entity victim, k = weapontype, l = colpoint
    // bullets can be fired without char, and without entity and colpoint collision (e.g. sky), so, consider it!
    m = -1
    IF i > 0
        GET_ENTITY_TYPE i m
        IF m = ENTITY_TYPE_PED
            GET_PED_REF i i
            GET_CHAR_MODEL i m
        ENDIF
    ENDIF
    IF j > 0
        READ_STRUCT_OFFSET j 0x22 2 n //CEntity->m_nModelIndex
        //GET_COLPOINT_SURFACE l o
        //GET_COLPOINT_COORDINATES l x y z
        // alternative way to read a vector3:
        READ_STRUCT_OFFSET_MULTI l 0x0 3 4 (x y z) //colpoint.point.x, y, z
        PRINT_FORMATTED_NOW "Bullet impact: cmodel %i emodel %i weap %i colppos %.2f %.2f %.2f" 1000 m n k x y z
    ENDIF
    RETURN_SCRIPT_EVENT

    EventOnMenu:
    IF eventArgVar = TRUE // just paused
        PRINT_STRING_NOW "Paused" 1000
    ELSE
        // This runs every frame on menu
    ENDIF
    RETURN_SCRIPT_EVENT

    EventCharProcess:
    GET_CHAR_COORDINATES eventArgVar (x y z)   // NOTE: These vars will overwrite if we are using somewhere else, caution!
    DRAW_SHADOW 3 x y z 0.0 1.5 255 255 0 0
    RETURN_SCRIPT_EVENT

    EventCarProcess:
    GET_CAR_COORDINATES eventArgVar (x y z)
    DRAW_SHADOW 3 x y z 0.0 2.5 255 0 255 0
    RETURN_SCRIPT_EVENT

    EventObjectProcess:
    GET_OBJECT_COORDINATES eventArgVar (x y z)
    DRAW_SHADOW 3 x y z 0.0 1.5 255 0 0 255
    RETURN_SCRIPT_EVENT

    EventBuildingProcess:
    // eventArgVar is CBuilding/CEntity
    GET_ENTITY_COORDINATES eventArgVar x y z
    DRAW_CORONA x y z 3.0 CORONATYPE_SHINYSTAR 0 255 0 255
    z += 4.0
    DRAW_SHADOW 3 x y z 0.0 4.0 255 255 0 255
    RETURN_SCRIPT_EVENT
    
    EventCharCreate_StartInternalThread:
    STREAM_CUSTOM_SCRIPT_FROM_LABEL CharInternalThread eventArgVar
    RETURN_SCRIPT_EVENT

    EventCarCreate_StartInternalThread:
    STREAM_CUSTOM_SCRIPT_FROM_LABEL CarInternalThread eventArgVar
    RETURN_SCRIPT_EVENT

    // We need to do it during ped process because during script process doesn't work correctly
    SimulateWeaponFire:
    IF n = scplayer
        IF IS_KEY_JUST_PRESSED VK_KEY_Y
            IF GET_CURRENT_CHAR_WEAPONINFO n (i)

                GET_PED_POINTER n k

                // Check if weapon state isn't reloading etc
                // We need to set weapon state too: ped->m_aWeapons[owner->m_nActiveWeaponSlot].m_nState = 1
                GET_WEAPONINFO_SLOT i l
                l *= 0x1C //CWeapon in slot array
                l += 0x5A4 // + ped->m_aWeapons + m_nState
                READ_STRUCT_OFFSET k l 4 m
                IF m = 0
                    WRITE_STRUCT_OFFSET k l 4 1 // set to shooting

                    // This will create a "fake" CTaskSimpleUseGun, include CWeaponInfo at 0x30 and set hand flags
                    GET_LABEL_POINTER Buffer128 j
                    WRITE_STRUCT_OFFSET j 0x30 4 i
                    WRITE_STRUCT_OFFSET j 0xD 1 1 // hand: 1 = left, 2 = right, 3 = left + right

                    // Call 0x00624ED0 bool __thiscall CTaskSimpleUseGun::SetPedPosition(struct CTaskSimpleUseGun *this, struct CPed *ped)
                    READ_MEMORY 0x86D744 4 FALSE i // get pointer to make it compatible with redirected calls, like MixSets and IMFX
                    CALL_METHOD i j 1 0 (k)
                ENDIF

            ENDIF
        ENDIF
    ENDIF
    RETURN_SCRIPT_EVENT
}
SCRIPT_END

{
    LVAR_INT char // In
    LVAR_FLOAT x y z
    CharInternalThread:
    PRINT_FORMATTED "Char create in internal thread: %i" 200 char
    WHILE DOES_CHAR_EXIST char
        GET_CHAR_COORDINATES char (x y z)
        DRAW_SHADOW 3 x y z 0.0 1.5 255 255 0 0
        WAIT 0
    ENDWHILE
    TERMINATE_THIS_CUSTOM_SCRIPT
}

{
    LVAR_INT car // In
    LVAR_FLOAT x y z
    CarInternalThread:
    PRINT_FORMATTED "Car create in internal thread: %i" 200 car
    WHILE DOES_VEHICLE_EXIST car
        GET_CAR_COORDINATES car (x y z)
        DRAW_SHADOW 3 x y z 0.0 1.5 255 0 255 0
        WAIT 0
    ENDWHILE
    TERMINATE_THIS_CUSTOM_SCRIPT
}

Buffer128:
DUMP
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //32
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //64
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //96
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //128
ENDDUMP

Buffer128B:
DUMP
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //32
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //64
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //96
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 //128
ENDDUMP
