#pragma once

#include "common.h"

#pragma warning(disable : 4480)

namespace iw3
{
    namespace mp
    {
        // TODO: verify fields
        struct DxGlobals
        {
            HINSTANCE__ *hinst;
            Direct3D *d3d9;
            D3DDevice *device;
            unsigned int adapterIndex;
            _D3DFORMAT depthStencilFormat;
            _D3DMULTISAMPLE_TYPE multiSampleType;
            unsigned int multiSampleQuality;
            D3DSurface *singleSampleDepthStencilSurface;
            D3DTexture *frontBufferTexture[2];
            unsigned int frontBufferTextureIndex;
            _D3DTEXTUREFILTERTYPE linearNonMippedMinFilter;
            _D3DTEXTUREFILTERTYPE linearNonMippedMagFilter;
            _D3DTEXTUREFILTERTYPE linearMippedMinFilter;
            _D3DTEXTUREFILTERTYPE linearMippedMagFilter;
            _D3DTEXTUREFILTERTYPE anisotropicMinFilter;
            _D3DTEXTUREFILTERTYPE anisotropicMagFilter;
            int linearMippedAnisotropy;
            int anisotropyFor2x;
            int anisotropyFor4x;
            int mipFilterMode;
            unsigned int mipBias;
            _D3DRING_BUFFER_PARAMETERS ringBufferParameters;
            volatile int showDirtyDiscError;
        };

        struct menuDef_t;

        struct UiContext_cursor
        {
            float x;
            float y;
        };
        static_assert(sizeof(UiContext_cursor) == 0x8, "");

        union UILocalVar_u
        {
            int integer;
            float value;
            const char *string;
        };

        enum UILocalVarType : __int32
        {
            UILOCALVAR_INT = 0x0,
            UILOCALVAR_FLOAT = 0x1,
            UILOCALVAR_STRING = 0x2,
        };

        struct UILocalVar
        {
            UILocalVarType type;
            const char *name;
            UILocalVar_u u;
        };

        struct UILocalVarContext
        {
            UILocalVar table[256];
        };

        struct UiContext
        {
            int localClientNum;
            float bias;
            int realTime;
            int frameTime;
            UiContext_cursor cursor;
            int isCursorVisible;
            int screenWidth;
            int screenHeight;
            float screenAspect;
            float FPS;
            float blurRadiusOut;
            menuDef_t *Menus[512];
            int menuCount;
            menuDef_t *menuStack[16];
            int openMenuCount;
            UILocalVarContext localVars;
        };
        static_assert(sizeof(UiContext) == 0x1478, "");

        enum uiMenuCommand_t : __int32
        {
            UIMENU_NONE = 0x0,
            UIMENU_MAIN = 0x1,
            UIMENU_INGAME = 0x2,
            UIMENU_PREGAME = 0x3,
            UIMENU_POSTGAME = 0x4,
            UIMENU_WM_QUICKMESSAGE = 0x5,
            UIMENU_SCRIPT_POPUP = 0x6,
            UIMENU_SCOREBOARD = 0x7,
            UIMENU_SPLITSCREENGAMESETUP = 0x8,
            UIMENU_SYSTEMLINKJOINGAME = 0x9,
            UIMENU_PARTY = 0xA,
            UIMENU_GAMELOBBY = 0xB,
            UIMENU_PRIVATELOBBY = 0xC,
        };

        struct __declspec(align(4)) uiInfo_s
        {
            UiContext uiDC;
            int myTeamCount;
            int playerRefresh;
            int playerIndex;
            int timeIndex;
            int previousTimes[4];
            uiMenuCommand_t currentMenuType;
            bool allowScriptMenuResponse;
        };
        static_assert(sizeof(uiInfo_s) == 5280, "");

        /* 9096 */
        struct EntHandle
        {
            unsigned __int16 number;
            unsigned __int16 infoIndex;
        };

        /* 9097 */
        struct entityShared_t
        {
            unsigned __int8 linked;
            unsigned __int8 bmodel;
            unsigned __int8 svFlags;
            int clientMask[2];
            unsigned __int8 inuse;
            int broadcastTime;
            float mins[3];
            float maxs[3];
            int contents;
            float absmin[3];
            float absmax[3];
            float currentOrigin[3];
            float currentAngles[3];
            EntHandle ownerNum;
            int eventTime;
        };

        static_assert(sizeof(entityShared_t) == 0x0068, "");

        /* 662 */
        enum OffhandSecondaryClass : __int32
        {
            PLAYER_OFFHAND_SECONDARY_SMOKE = 0x0,
            PLAYER_OFFHAND_SECONDARY_FLASH = 0x1,
            PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
        };

        /* 663 */
        enum ViewLockTypes : __int32
        {
            PLAYERVIEWLOCK_NONE = 0x0,
            PLAYERVIEWLOCK_FULL = 0x1,
            PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
            PLAYERVIEWLOCKCOUNT = 0x3,
        };

        /* 665 */
        enum team_t : __int32
        {
            TEAM_FREE = 0x0,
            TEAM_AXIS = 0x1,
            TEAM_ALLIES = 0x2,
            TEAM_SPECTATOR = 0x3,
            TEAM_NUM_TEAMS = 0x4,
        };

        /* 8733 */
        struct SprintState
        {
            int sprintButtonUpRequired;
            int sprintDelay;
            int lastSprintStart;
            int lastSprintEnd;
            int sprintStartMaxLength;
        };

        /* 8734 */
        struct MantleState
        {
            float yaw;
            int timer;
            int transIndex;
            int flags;
        };

        /* 664 */
        enum ActionSlotType : __int32
        {
            ACTIONSLOTTYPE_DONOTHING = 0x0,
            ACTIONSLOTTYPE_SPECIFYWEAPON = 0x1,
            ACTIONSLOTTYPE_ALTWEAPONTOGGLE = 0x2,
            ACTIONSLOTTYPE_NIGHTVISION = 0x3,
            ACTIONSLOTTYPECOUNT = 0x4,
        };

        /* 8721 */
        struct ActionSlotParam_SpecifyWeapon
        {
            unsigned int index;
        };

        /* 8735 */
        struct ActionSlotParam
        {
            ActionSlotParam_SpecifyWeapon specifyWeapon;
        };

        /* 660 */
        enum objectiveState_t : __int32
        {
            OBJST_EMPTY = 0x0,
            OBJST_ACTIVE = 0x1,
            OBJST_INVISIBLE = 0x2,
            OBJST_DONE = 0x3,
            OBJST_CURRENT = 0x4,
            OBJST_FAILED = 0x5,
            OBJST_NUMSTATES = 0x6,
        };

        /* 8736 */
        struct objective_t
        {
            objectiveState_t state;
            float origin[3];
            int entNum;
            int teamNum;
            int icon;
        };

        /* 667 */
        enum he_type_t : __int32
        {
            HE_TYPE_FREE = 0x0,
            HE_TYPE_TEXT = 0x1,
            HE_TYPE_VALUE = 0x2,
            HE_TYPE_PLAYERNAME = 0x3,
            HE_TYPE_MAPNAME = 0x4,
            HE_TYPE_GAMETYPE = 0x5,
            HE_TYPE_MATERIAL = 0x6,
            HE_TYPE_TIMER_DOWN = 0x7,
            HE_TYPE_TIMER_UP = 0x8,
            HE_TYPE_TENTHS_TIMER_DOWN = 0x9,
            HE_TYPE_TENTHS_TIMER_UP = 0xA,
            HE_TYPE_CLOCK_DOWN = 0xB,
            HE_TYPE_CLOCK_UP = 0xC,
            HE_TYPE_WAYPOINT = 0xD,
            HE_TYPE_COUNT = 0xE,
        };

        /* 8713 */
        struct $0D0CB43DF22755AD856C77DD3F304010
        {
            unsigned __int8 r;
            unsigned __int8 g;
            unsigned __int8 b;
            unsigned __int8 a;
        };

        /* 8714 */
        union hudelem_color_t
        {
            $0D0CB43DF22755AD856C77DD3F304010 __s0;
            int rgba;
        };

        /* 8737 */
        struct hudelem_s
        {
            he_type_t type;
            float x;
            float y;
            float z;
            int targetEntNum;
            float fontScale;
            int font;
            int alignOrg;
            int alignScreen;
            hudelem_color_t color;
            hudelem_color_t fromColor;
            int fadeStartTime;
            int fadeTime;
            int label;
            int width;
            int height;
            int materialIndex;
            int offscreenMaterialIdx;
            int fromWidth;
            int fromHeight;
            int scaleStartTime;
            int scaleTime;
            float fromX;
            float fromY;
            int fromAlignOrg;
            int fromAlignScreen;
            int moveStartTime;
            int moveTime;
            int time;
            int duration;
            float value;
            int text;
            float sort;
            hudelem_color_t glowColor;
            int fxBirthTime;
            int fxLetterTime;
            int fxDecayStartTime;
            int fxDecayDuration;
            int soundID;
            int flags;
        };

        struct hudElemState_t
        {
            hudelem_s current[31];
            hudelem_s archival[31];
        };

        enum weaponstate_t : __int32
        {
            WEAPON_READY = 0x0,
            WEAPON_RAISING = 0x1,
            WEAPON_RAISING_ALTSWITCH = 0x2,
            WEAPON_DROPPING = 0x3,
            WEAPON_DROPPING_QUICK = 0x4,
            WEAPON_FIRING = 0x5,
            WEAPON_RECHAMBERING = 0x6,
            WEAPON_RELOADING = 0x7,
            WEAPON_RELOADING_INTERUPT = 0x8,
            WEAPON_RELOAD_START = 0x9,
            WEAPON_RELOAD_START_INTERUPT = 0xA,
            WEAPON_RELOAD_END = 0xB,
            WEAPON_MELEE_INIT = 0xC,
            WEAPON_MELEE_FIRE = 0xD,
            WEAPON_MELEE_END = 0xE,
            WEAPON_OFFHAND_INIT = 0xF,
            WEAPON_OFFHAND_PREPARE = 0x10,
            WEAPON_OFFHAND_HOLD = 0x11,
            WEAPON_OFFHAND_START = 0x12,
            WEAPON_OFFHAND = 0x13,
            WEAPON_OFFHAND_END = 0x14,
            WEAPON_DETONATING = 0x15,
            WEAPON_SPRINT_RAISE = 0x16,
            WEAPON_SPRINT_LOOP = 0x17,
            WEAPON_SPRINT_DROP = 0x18,
            WEAPON_NIGHTVISION_WEAR = 0x19,
            WEAPON_NIGHTVISION_REMOVE = 0x1A,
            WEAPONSTATES_NUM = 0x1B,
        };

        struct playerState_s
        {
            int commandTime; // 0
            int pm_type;     // 4
            int bobCycle;    // 8
            int pm_flags;    // 12
            int weapFlags;
            int otherFlags;
            int pm_time;
            float origin[3];
            float velocity[3]; // 40
            float oldVelocity[2];
            int weaponTime;
            int weaponDelay;
            int grenadeTimeLeft;
            int throwBackGrenadeOwner;
            int throwBackGrenadeTimeLeft;
            int weaponRestrictKickTime;
            int foliageSoundTime;
            int gravity;
            float leanf;
            int speed;
            float delta_angles[3];
            int groundEntityNum; // 112
            float vLadderVec[3];
            int jumpTime;
            float jumpOriginZ; // 132
            int legsTimer;     // 136
            int legsAnim;      // 140
            int torsoTimer;    // 144
            int torsoAnim;
            int legsAnimDuration;
            int torsoAnimDuration;
            int damageTimer;
            int damageDuration;
            int flinchYawAnim; // 168
            char _pad[4];      // 172
            int movementDir;   // 176
            int eFlags;        // 180
            int eventSequence;
            int events[4];
            unsigned int eventParms[4];
            int oldEventSequence;
            int clientNum;
            int offHandIndex;
            OffhandSecondaryClass offhandSecondary;
            unsigned int weapon;
            weaponstate_t weaponstate; // 240
            unsigned int weaponShotCount;
            float fWeaponPosFrac;
            int adsDelayTime;
            int spreadOverride;
            int spreadOverrideState;
            int viewmodelIndex;
            float viewangles[3]; // 268
            int viewHeightTarget;
            float viewHeightCurrent;
            int viewHeightLerpTime;
            int viewHeightLerpTarget;
            int viewHeightLerpDown;
            float viewAngleClampBase[2];
            float viewAngleClampRange[2];
            int damageEvent;
            int damageYaw;
            int damagePitch;
            int damageCount;
            int stats[5];
            int ammo[128];
            int ammoclip[128];
            unsigned int weapons[4];
            unsigned int weaponold[4];
            unsigned int weaponrechamber[4];
            float proneDirection;
            float proneDirectionPitch;
            float proneTorsoPitch;
            ViewLockTypes viewlocked;
            int viewlocked_entNum;
            int cursorHint;
            int cursorHintString;
            int cursorHintEntIndex;
            int iCompassPlayerInfo;
            int radarEnabled;
            int locationSelectionInfo;
            SprintState sprintState;
            float fTorsoPitch;
            float fWaistPitch;
            float holdBreathScale;
            int holdBreathTimer;
            float moveSpeedScaleMultiplier;
            MantleState mantleState;
            float meleeChargeYaw;
            int meleeChargeDist;
            int meleeChargeTime;
            int perks;
            ActionSlotType actionSlotType[4];
            ActionSlotParam actionSlotParam[4];
            int entityEventSequence;
            int weapAnim;
            float aimSpreadScale;
            int shellshockIndex;
            int shellshockTime;
            int shellshockDuration;
            float dofNearStart;
            float dofNearEnd;
            float dofFarStart;
            float dofFarEnd;
            float dofNearBlur;
            float dofFarBlur;
            float dofViewmodelStart;
            float dofViewmodelEnd;
            int hudElemLastAssignedSoundID;
            objective_t objective[16];
            unsigned __int8 weaponmodels[128];
            int deltaTime;
            int killCamEntity;
            hudElemState_t hud;

            inline bool isOnGround()
            {
                return groundEntityNum == 1022;
            }

            inline bool isInAir()
            {
                return groundEntityNum == 1023;
            }

            inline bool isReloading()
            {
                return weaponstate == WEAPON_RELOADING;
            }

            inline bool holdingWeapon(unsigned int weaponIndex)
            {
                return (weapon == weaponIndex);
            }
        };

        static_assert(offsetof(playerState_s, velocity) == 40, "");
        static_assert(offsetof(playerState_s, groundEntityNum) == 112, "");
        static_assert(offsetof(playerState_s, jumpOriginZ) == 132, "");
        static_assert(offsetof(playerState_s, legsTimer) == 136, "");
        static_assert(offsetof(playerState_s, torsoTimer) == 144, "");
        static_assert(offsetof(playerState_s, flinchYawAnim) == 168, "");
        static_assert(offsetof(playerState_s, movementDir) == 176, "");
        static_assert(offsetof(playerState_s, eFlags) == 180, "");
        static_assert(offsetof(playerState_s, weaponstate) == 240, "");
        static_assert(offsetof(playerState_s, viewangles) == 268, "");

        struct playerTeamState_t
        {
            int location;
        };

        /* 8741 */
        struct clientState_s
        {
            int clientIndex;
            team_t team;
            int modelindex;
            int attachModelIndex[6];
            int attachTagIndex[6];
            char name[32];
            float maxSprintTimeMultiplier;
            int rank;
            int prestige;
            int perks;
            int voiceConnectivityBits;
            char clanAbbrev[8];
            int attachedVehEntNum;
            int attachedVehSlotIndex;
        };

        /* 770 */
        enum clientConnected_t : __int32
        {
            CON_DISCONNECTED = 0x0,
            CON_CONNECTING = 0x1,
            CON_CONNECTED = 0x2,
        };

        /* 771 */
        enum sessionState_t : __int32
        {
            SESS_STATE_PLAYING = 0x0,
            SESS_STATE_DEAD = 0x1,
            SESS_STATE_SPECTATOR = 0x2,
            SESS_STATE_INTERMISSION = 0x3,
        };

        /* 8748 */
        struct __declspec(align(2)) usercmd_s
        {
            int serverTime;
            int buttons;
            int angles[3];
            unsigned __int8 weapon;
            unsigned __int8 offHandIndex;
            char forwardmove;
            char rightmove;
            float meleeChargeYaw;
            unsigned __int8 meleeChargeDist;
            char selectedLocation[2];
        };

        static_assert(sizeof(usercmd_s) == 32, "");

        /* 9100 */
        struct clientSession_t
        {
            sessionState_t sessionState; // correct
            int forceSpectatorClient;
            int killCamEntity;
            int status_icon;
            int archiveTime;
            int score;
            int deaths;
            int kills;
            int assists;
            unsigned __int16 scriptPersId;
            clientConnected_t connected;
            usercmd_s cmd;
            usercmd_s oldcmd;
            int localClient;
            int predictItemPickup;
            char newnetname[32];
            int maxHealth;
            int enterTime;
            playerTeamState_t teamState;
            int voteCount;
            int teamVoteCount;
            float moveSpeedScaleMultiplier;
            int viewmodelIndex;
            int noSpectate;
            int teamInfo;
            clientState_s cs;
            int psOffsetTime;
        };

        struct gentity_s;

        struct gclient_s
        {
            playerState_s ps;
            clientSession_t sess;
            int spectatorClient;
            int noclip; // 0x30a8
            int ufo;    // 0x30ac
            int bFrozen;
            int lastCmdTime;
            int buttons;
            int oldbuttons;
            int latched_buttons;
            int buttonsSinceLastFrame;
            float oldOrigin[3];
            float fGunPitch;
            float fGunYaw;
            int damage_blood;
            float damage_from[3];
            int damage_fromWorld;
            int accurateCount;
            int accuracy_shots;
            int accuracy_hits;
            int inactivityTime;
            int inactivityWarning;
            int lastVoiceTime;
            int switchTeamTime;
            float currentAimSpreadScale;
            gentity_s *persistantPowerup;
            int portalID;
            int dropWeaponTime;
            int sniperRifleFiredTime;
            float sniperRifleMuzzleYaw;
            int PCSpecialPickedUpCount;
            EntHandle useHoldEntity;
            int useHoldTime;
            int useButtonDone;
            int iLastCompassPlayerInfoEnt;
            int compassPingTime;
            int damageTime;
            float v_dmg_roll;
            float v_dmg_pitch;
            float swayViewAngles[3];
            float swayOffset[3];
            float swayAngles[3];
            float vLastMoveAng[3];
            float fLastIdleFactor;
            float vGunOffset[3];
            float vGunSpeed[3];
            int weapIdleTime;
            int lastServerTime;
            int lastSpawnTime;
            unsigned int lastWeapon;
            bool previouslyFiring;
            bool previouslyUsingNightVision;
            bool previouslySprinting;
            int hasRadar;
            int lastStand;
            int lastStandTime;
        };

        static_assert(offsetof(gclient_s, noclip) == 0x30a8, "");
        static_assert(offsetof(gclient_s, ufo) == 0x30ac, "");
        static_assert(sizeof(gclient_s) == 12724, "");

        static_assert(offsetof(gclient_s, sess) + offsetof(clientSession_t, cmd) == 12180, "");
        static_assert(offsetof(gclient_s, sess) + offsetof(clientSession_t, archiveTime) == 12152, "");

        struct LerpEntityStatePhysicsJitter
        {
            float innerRadius;
            float minDisplacement;
            float maxDisplacement;
        };

        struct LerpEntityStatePlayer
        {
            float leanf;
            int movementDir;
        };

        struct LerpEntityStateLoopFx
        {
            float cullDist;
            int period;
        };

        struct LerpEntityStateCustomExplode
        {
            int startTime;
        };

        struct LerpEntityStateTurret
        {
            float gunAngles[3];
        };

        struct LerpEntityStateAnonymous
        {
            int data[7];
        };

        struct LerpEntityStateExplosion
        {
            float innerRadius;
            float magnitude;
        };

        struct LerpEntityStateBulletHit
        {
            float start[3];
        };

        struct LerpEntityStatePrimaryLight
        {
            byte colorAndExp[4];
            float intensity;
            float radius;
            float cosHalfFovOuter;
            float cosHalfFovInner;
        };

        struct LerpEntityStateMissile
        {
            int launchTime;
        };

        struct LerpEntityStateSoundBlend
        {
            float lerp;
        };

        struct LerpEntityStateExplosionJolt
        {
            float innerRadius;
            float impulse[3];
        };

        struct LerpEntityStateVehicle
        {
            float bodyPitch;
            float bodyRoll;
            float steerYaw;
            int materialTime;
            float gunPitch;
            float gunYaw;
            int team;
        };

        struct LerpEntityStateEarthquake
        {
            float scale;
            float radius;
            int duration;
        };

        /* 678 */
        enum trType_t : __int32
        {
            TR_STATIONARY = 0x0,
            TR_INTERPOLATE = 0x1,
            TR_LINEAR = 0x2,
            TR_LINEAR_STOP = 0x3,
            TR_SINE = 0x4,
            TR_GRAVITY = 0x5,
            TR_ACCELERATE = 0x6,
            TR_DECELERATE = 0x7,
            TR_PHYSICS = 0x8,
            TR_FIRST_RAGDOLL = 0x9,
            TR_RAGDOLL = 0x9,
            TR_RAGDOLL_GRAVITY = 0xA,
            TR_RAGDOLL_INTERPOLATE = 0xB,
            TR_LAST_RAGDOLL = 0xB,
        };

        /* 8750 */
        struct trajectory_t
        {
            trType_t trType;
            int trTime;
            int trDuration;
            float trBase[3];
            float trDelta[3];
        };

        /* 8743 */
        union LerpEntityStateTypeUnion
        {
            LerpEntityStateTurret turret;
            LerpEntityStateLoopFx loopFx;
            LerpEntityStatePrimaryLight primaryLight;
            LerpEntityStatePlayer player;
            LerpEntityStateVehicle vehicle;
            LerpEntityStateMissile missile;
            LerpEntityStateSoundBlend soundBlend;
            LerpEntityStateBulletHit bulletHit;
            LerpEntityStateEarthquake earthquake;
            LerpEntityStateCustomExplode customExplode;
            LerpEntityStateExplosion explosion;
            LerpEntityStateExplosionJolt explosionJolt;
            LerpEntityStatePhysicsJitter physicsJitter;
            LerpEntityStateAnonymous anonymous;
        };

        /* 8751 */
        struct LerpEntityState
        {
            int eFlags;
            trajectory_t pos;
            trajectory_t apos;
            LerpEntityStateTypeUnion u;
        };

        struct entityState_s
        {
            int number; // entity index	//0x00
            int eType;  // entityType_t	//0x04

            LerpEntityState lerp;

            int time2; // 0x70

            int otherEntityNum;    // 0x74 shotgun sources, etc
            int attackerEntityNum; // 0x78

            int groundEntityNum; // 0x7c -1 = in air

            int loopSound; // 0x80 constantly loop this sound
            int surfType;  // 0x84

            int index;         // 0x88
            int clientNum;     // 0x8c 0 to (MAX_CLIENTS - 1), for players and corpses
            int iHeadIcon;     // 0x90
            int iHeadIconTeam; // 0x94

            int solid; // 0x98 for client side prediction, trap_linkentity sets this properly	0x98

            int eventParm;     // 0x9c impulse events -- muzzle flashes, footsteps, etc
            int eventSequence; // 0xa0

            int events[4];     // 0xa4
            int eventParms[4]; // 0xb4

            // for players
            int weapon;      // 0xc4 determines weapon and flash model, etc
            int weaponModel; // 0xc8
            int legsAnim;    // 0xcc mask off ANIM_TOGGLEBIT
            int torsoAnim;   // 0xd0 mask off ANIM_TOGGLEBIT

            union
            {
                int helicopterStage; // 0xd4
            } un1;

            int un2;                  // 0xd8
            int fTorsoPitch;          // 0xdc
            int fWaistPitch;          // 0xe0
            unsigned int partBits[4]; // 0xe4
        };

        static_assert(sizeof(entityState_s) == 0xf4, "");
        static_assert(offsetof(entityState_s, index) == 0x88, "");

        struct turretInfo_s;
        struct scr_vehicle_s;

        struct item_ent_t
        {
            int ammoCount;
            int clipAmmoCount;
            int index;
        };

        struct __declspec(align(4)) trigger_ent_t
        {
            int threshold;
            int accumulate;
            int timestamp;
            int singleUserEntIndex;
            bool requireLookAt;
        };

        struct mover_ent_t
        {
            float decelTime;
            float aDecelTime;
            float speed;
            float aSpeed;
            float midTime;
            float aMidTime;
            float pos1[3];
            float pos2[3];
            float pos3[3];
            float apos1[3];
            float apos2[3];
            float apos3[3];
        };

        struct corpse_ent_t
        {
            int deathAnimStartTime;
        };

        enum MissileStage : __int32
        {
            MISSILESTAGE_SOFTLAUNCH = 0x0,
            MISSILESTAGE_ASCENT = 0x1,
            MISSILESTAGE_DESCENT = 0x2,
        };

        enum MissileFlightMode : __int32
        {
            MISSILEFLIGHTMODE_TOP = 0x0,
            MISSILEFLIGHTMODE_DIRECT = 0x1,
        };

        struct missile_ent_t
        {
            float time;
            int timeOfBirth;
            float travelDist;
            float surfaceNormal[3];
            team_t team;
            float curvature[3];
            float targetOffset[3];
            MissileStage stage;
            MissileFlightMode flightMode;
        };

        union $71462809E721C4770F13398DD6519E31
        {
            item_ent_t item[2];
            trigger_ent_t trigger;
            mover_ent_t mover;
            corpse_ent_t corpse;
            missile_ent_t missile;
        };

        struct tagInfo_s;

        struct gentity_s
        {
            entityState_s s;
            entityShared_t r;
            gclient_s *client;
            turretInfo_s *pTurretInfo;
            scr_vehicle_s *scr_vehicle;
            unsigned __int16 model;
            unsigned __int8 physicsObject;
            unsigned __int8 takedamage;
            unsigned __int8 active;
            unsigned __int8 nopickup;
            unsigned __int8 handler;
            unsigned __int8 team;
            unsigned __int16 classname;
            unsigned __int16 target;
            unsigned __int16 targetname;
            unsigned int attachIgnoreCollision;
            int spawnflags;
            int flags;
            int eventTime;
            int freeAfterEvent;
            int unlinkAfterEvent;
            int clipmask;
            int processedFrame;
            EntHandle parent;
            int nextthink;
            int health;
            int maxHealth;
            int damage;
            int count;
            gentity_s *chain;
            $71462809E721C4770F13398DD6519E31 ___u30;
            EntHandle missileTargetEnt;
            tagInfo_s *tagInfo;
            gentity_s *tagChildren;
            unsigned __int16 attachModelNames[19];
            unsigned __int16 attachTagNames[19];
            int useCount;
            gentity_s *nextFree;
            char _pad[4]; // TODO: find correct padding position
        };

        static_assert(offsetof(gentity_s, client) == 0x0015C, "");
        static_assert(sizeof(gentity_s) == 0x278, "");

        struct cmd_function_s
        {
            cmd_function_s *next;
            const char *name;
            const char *autoCompleteDir;
            const char *autoCompleteExt;
            void(__fastcall *function)();
        };

        enum keyNum_t : __int32
        {
            K_NONE = 0x0,
            K_FIRSTGAMEPADBUTTON_RANGE_1 = 0x1,
            K_BUTTON_A = 0x1,
            K_BUTTON_B = 0x2,
            K_BUTTON_X = 0x3,
            K_BUTTON_Y = 0x4,
            K_BUTTON_LSHLDR = 0x5,
            K_BUTTON_RSHLDR = 0x6,
            K_LASTGAMEPADBUTTON_RANGE_1 = 0x6,
            K_TAB = 0x9,
            K_ENTER = 0xD,
            K_FIRSTGAMEPADBUTTON_RANGE_2 = 0xE,
            K_BUTTON_START = 0xE,
            K_BUTTON_BACK = 0xF,
            K_BUTTON_LSTICK = 0x10,
            K_BUTTON_RSTICK = 0x11,
            K_BUTTON_LTRIG = 0x12,
            K_BUTTON_RTRIG = 0x13,
            K_DPAD_UP = 0x14,
            K_FIRSTDPAD = 0x14,
            K_DPAD_DOWN = 0x15,
            K_DPAD_LEFT = 0x16,
            K_DPAD_RIGHT = 0x17,
            K_BUTTON_LSTICK_ALTIMAGE = 0xBC,
            K_BUTTON_RSTICK_ALTIMAGE = 0xBD,
            K_LASTDPAD = 0x17,
            K_LASTGAMEPADBUTTON_RANGE_2 = 0x17,
            K_ESCAPE = 0x1B,
            K_FIRSTGAMEPADBUTTON_RANGE_3 = 0x1C,
            K_APAD_UP = 0x1C,
            K_FIRSTAPAD = 0x1C,
            K_APAD_DOWN = 0x1D,
            K_APAD_LEFT = 0x1E,
            K_APAD_RIGHT = 0x1F,
            K_LASTAPAD = 0x1F,
            K_LASTGAMEPADBUTTON_RANGE_3 = 0x1F,
            K_SPACE = 0x20,
            K_BACKSPACE = 0x7F,
            K_ASCII_FIRST = 0x80,
            K_ASCII_181 = 0x80,
            K_ASCII_191 = 0x81,
            K_ASCII_223 = 0x82,
            K_ASCII_224 = 0x83,
            K_ASCII_225 = 0x84,
            K_ASCII_228 = 0x85,
            K_ASCII_229 = 0x86,
            K_ASCII_230 = 0x87,
            K_ASCII_231 = 0x88,
            K_ASCII_232 = 0x89,
            K_ASCII_233 = 0x8A,
            K_ASCII_236 = 0x8B,
            K_ASCII_241 = 0x8C,
            K_ASCII_242 = 0x8D,
            K_ASCII_243 = 0x8E,
            K_ASCII_246 = 0x8F,
            K_ASCII_248 = 0x90,
            K_ASCII_249 = 0x91,
            K_ASCII_250 = 0x92,
            K_ASCII_252 = 0x93,
            K_END_ASCII_CHARS = 0x94,
            K_COMMAND = 0x96,
            K_CAPSLOCK = 0x97,
            K_POWER = 0x98,
            K_PAUSE = 0x99,
            K_UPARROW = 0x9A,
            K_DOWNARROW = 0x9B,
            K_LEFTARROW = 0x9C,
            K_RIGHTARROW = 0x9D,
            K_ALT = 0x9E,
            K_CTRL = 0x9F,
            K_SHIFT = 0xA0,
            K_INS = 0xA1,
            K_DEL = 0xA2,
            K_PGDN = 0xA3,
            K_PGUP = 0xA4,
            K_HOME = 0xA5,
            K_END = 0xA6,
            K_F1 = 0xA7,
            K_F2 = 0xA8,
            K_F3 = 0xA9,
            K_F4 = 0xAA,
            K_F5 = 0xAB,
            K_F6 = 0xAC,
            K_F7 = 0xAD,
            K_F8 = 0xAE,
            K_F9 = 0xAF,
            K_F10 = 0xB0,
            K_F11 = 0xB1,
            K_F12 = 0xB2,
            K_F13 = 0xB3,
            K_F14 = 0xB4,
            K_F15 = 0xB5,
            K_KP_HOME = 0xB6,
            K_KP_UPARROW = 0xB7,
            K_KP_PGUP = 0xB8,
            K_KP_LEFTARROW = 0xB9,
            K_KP_5 = 0xBA,
            K_KP_RIGHTARROW = 0xBB,
            K_KP_END = 0xBC,
            K_KP_DOWNARROW = 0xBD,
            K_KP_PGDN = 0xBE,
            K_KP_ENTER = 0xBF,
            K_KP_INS = 0xC0,
            K_KP_DEL = 0xC1,
            K_KP_SLASH = 0xC2,
            K_KP_MINUS = 0xC3,
            K_KP_PLUS = 0xC4,
            K_KP_NUMLOCK = 0xC5,
            K_KP_STAR = 0xC6,
            K_KP_EQUALS = 0xC7,
            K_MOUSE1 = 0xC8,
            K_MOUSE2 = 0xC9,
            K_MOUSE3 = 0xCA,
            K_MOUSE4 = 0xCB,
            K_MOUSE5 = 0xCC,
            K_MWHEELDOWN = 0xCD,
            K_MWHEELUP = 0xCE,
            K_AUX1 = 0xCF,
            K_AUX2 = 0xD0,
            K_AUX3 = 0xD1,
            K_AUX4 = 0xD2,
            K_AUX5 = 0xD3,
            K_AUX6 = 0xD4,
            K_AUX7 = 0xD5,
            K_AUX8 = 0xD6,
            K_AUX9 = 0xD7,
            K_AUX10 = 0xD8,
            K_AUX11 = 0xD9,
            K_AUX12 = 0xDA,
            K_AUX13 = 0xDB,
            K_AUX14 = 0xDC,
            K_AUX15 = 0xDD,
            K_AUX16 = 0xDE,
            K_LAST_KEY = 0xDF,
        };

        struct MapEnts
        {
            const char *name;
            char *entityString;
            int numEntityChars;
        };

        enum XAssetType : __int32
        {
            ASSET_TYPE_XMODELPIECES = 0x0,
            ASSET_TYPE_PHYSPRESET = 0x1,
            ASSET_TYPE_XANIMPARTS = 0x2,
            ASSET_TYPE_XMODEL = 0x3,
            ASSET_TYPE_MATERIAL = 0x4,
            ASSET_TYPE_PIXELSHADER = 0x5,
            ASSET_TYPE_TECHNIQUE_SET = 0x6,
            ASSET_TYPE_IMAGE = 0x7,
            ASSET_TYPE_SOUND = 0x8,
            ASSET_TYPE_SOUND_CURVE = 0x9,
            ASSET_TYPE_LOADED_SOUND = 0xA,
            ASSET_TYPE_CLIPMAP = 0xB,
            ASSET_TYPE_CLIPMAP_PVS = 0xC,
            ASSET_TYPE_COMWORLD = 0xD,
            ASSET_TYPE_GAMEWORLD_SP = 0xE,
            ASSET_TYPE_GAMEWORLD_MP = 0xF,
            ASSET_TYPE_MAP_ENTS = 0x10,
            ASSET_TYPE_GFXWORLD = 0x11,
            ASSET_TYPE_LIGHT_DEF = 0x12,
            ASSET_TYPE_UI_MAP = 0x13,
            ASSET_TYPE_FONT = 0x14,
            ASSET_TYPE_MENULIST = 0x15,
            ASSET_TYPE_MENU = 0x16,
            ASSET_TYPE_LOCALIZE_ENTRY = 0x17,
            ASSET_TYPE_WEAPON = 0x18,
            ASSET_TYPE_SNDDRIVER_GLOBALS = 0x19,
            ASSET_TYPE_FX = 0x1A,
            ASSET_TYPE_IMPACT_FX = 0x1B,
            ASSET_TYPE_AITYPE = 0x1C,
            ASSET_TYPE_MPTYPE = 0x1D,
            ASSET_TYPE_CHARACTER = 0x1E,
            ASSET_TYPE_XMODELALIAS = 0x1F,
            ASSET_TYPE_RAWFILE = 0x20,
            ASSET_TYPE_STRINGTABLE = 0x21,
            ASSET_TYPE_COUNT = 0x22,
            ASSET_TYPE_STRING = 0x22,
            ASSET_TYPE_ASSETLIST = 0x23,
        };

        /* 760 */
        enum MapType : __int32
        {
            MAPTYPE_NONE = 0x0,
            MAPTYPE_INVALID1 = 0x1,
            MAPTYPE_INVALID2 = 0x2,
            MAPTYPE_2D = 0x3,
            MAPTYPE_3D = 0x4,
            MAPTYPE_CUBE = 0x5,
            MAPTYPE_COUNT = 0x6,
        };

        /* 8713 */
        union GfxTexture
        {
            D3DBaseTexture *basemap;
            D3DTexture *map;
            D3DVolumeTexture *volmap;
            D3DCubeTexture *cubemap;
            struct GfxImageLoadDef *loadDef; // forward declaration
        };

        /* 8714 */
        struct GfxImageLoadDef
        {
            unsigned __int8 levelCount;
            unsigned __int8 flags;
            __int16 dimensions[3];
            int format;
            GfxTexture texture;
        };

        /* 8715 */
        struct CardMemory
        {
            int platform[1];
        };

        /* 8716 */
        struct GfxImage
        {
            MapType mapType;
            GfxTexture texture;
            unsigned __int8 semantic;
            CardMemory cardMemory;
            unsigned __int16 width;
            unsigned __int16 height;
            unsigned __int16 depth;
            unsigned __int8 category;
            bool delayLoadPixels;
            unsigned __int8 *pixels;
            unsigned int baseSize;
            unsigned __int16 streamSlot;
            bool streaming;
            const char *name;
        };

        struct RawFile
        {
            const char *name;
            int len;
            const char *buffer;
        };

        struct Material;

        struct Glyph
        {
            unsigned __int16 letter;
            char x0;
            char y0;
            unsigned __int8 dx;
            unsigned __int8 pixelWidth;
            unsigned __int8 pixelHeight;
            float s0;
            float t0;
            float s1;
            float t1;
        };
        static_assert(sizeof(Glyph) == 0x18, "");

        struct Font_s
        {
            const char *fontName;
            int pixelHeight;
            int glyphCount;
            Material *material;
            Material *glowMaterial;
            Glyph *glyphs;
        };
        static_assert(sizeof(Font_s) == 0x18, "");

        struct XModel
        {
            const char *name;
        };

        union XAssetHeader
        {
            // XModelPieces *xmodelPieces;
            // PhysPreset *physPreset;
            // XAnimParts *parts;
            XModel *model;
            Material *material;
            // MaterialPixelShader *pixelShader;
            // MaterialVertexShader *vertexShader;
            // MaterialTechniqueSet *techniqueSet;
            GfxImage *image;
            // snd_alias_list_t *sound;
            // SndCurve *sndCurve;
            // LoadedSound *loadSnd;
            // clipMap_t *clipMap;
            // ComWorld *comWorld;
            // GameWorldSp *gameWorldSp;
            // GameWorldMp *gameWorldMp;
            // MapEnts *mapEnts;
            // GfxWorld *gfxWorld;
            // GfxLightDef *lightDef;
            Font_s *font;
            // MenuList *menuList;
            // menuDef_t *menu;
            // LocalizeEntry *localize;
            // WeaponDef *weapon;
            // SndDriverGlobals *sndDriverGlobals;
            // const FxEffectDef *fx;
            // FxImpactTable *impactFx;
            RawFile *rawfile;
            // StringTable *stringTable;
            void *data;
        };

        enum conChannel_t : __int32
        {
            CON_CHANNEL_DONT_FILTER = 0x0,
            CON_CHANNEL_ERROR = 0x1,
            CON_CHANNEL_GAMENOTIFY = 0x2,
            CON_CHANNEL_BOLDGAME = 0x3,
            CON_CHANNEL_SUBTITLE = 0x4,
            CON_CHANNEL_OBITUARY = 0x5,
            CON_CHANNEL_LOGFILEONLY = 0x6,
            CON_CHANNEL_CONSOLEONLY = 0x7,
            CON_CHANNEL_GFX = 0x8,
            CON_CHANNEL_SOUND = 0x9,
            CON_CHANNEL_FILES = 0xA,
            CON_CHANNEL_DEVGUI = 0xB,
            CON_CHANNEL_PROFILE = 0xC,
            CON_CHANNEL_UI = 0xD,
            CON_CHANNEL_CLIENT = 0xE,
            CON_CHANNEL_SERVER = 0xF,
            CON_CHANNEL_SYSTEM = 0x10,
            CON_CHANNEL_PLAYERWEAP = 0x11,
            CON_CHANNEL_AI = 0x12,
            CON_CHANNEL_ANIM = 0x13,
            CON_CHANNEL_PHYS = 0x14,
            CON_CHANNEL_FX = 0x15,
            CON_CHANNEL_LEADERBOARDS = 0x16,
            CON_CHANNEL_PARSERSCRIPT = 0x17,
            CON_CHANNEL_SCRIPT = 0x18,
            CON_BUILTIN_CHANNEL_COUNT = 0x19,
        };

        struct ImageList
        {
            unsigned int count;
            GfxImage *image[2048];
        };

        struct XAsset
        {
            XAssetType type;
            XAssetHeader header;
        };

        struct XAssetEntry
        {
            XAsset asset;
            unsigned __int8 zoneIndex;
            bool inuse;
            unsigned __int16 nextHash;
            unsigned __int16 nextOverride;
            unsigned __int16 usageFrame;
        };

        union XAssetEntryPoolEntry
        {
            XAssetEntry entry;
            XAssetEntryPoolEntry *next;
        };

        enum svscmd_type : __int32
        {
            SV_CMD_CAN_IGNORE = 0x0,
            SV_CMD_RELIABLE = 0x1,
        };

        /* 671 */
        enum netsrc_t : __int32
        {
            NS_CLIENT1 = 0x0,
            NS_CLIENT2 = 0x1,
            NS_CLIENT3 = 0x2,
            NS_CLIENT4 = 0x3,
            NS_SERVER = 0x4,
            NS_MAXCLIENTS = 0x4,
            NS_PACKET = 0x5,
        };

        /* 659 */
        enum netadrtype_t : __int32
        {
            NA_BOT = 0x0,
            NA_BAD = 0x1,
            NA_LOOPBACK = 0x2,
            NA_BROADCAST = 0x3,
            NA_IP = 0x4,
        };

        /* 8757 */
        struct __declspec(align(4)) netadr_t
        {
            netadrtype_t type;
            unsigned __int8 ip[4];
            unsigned __int16 port;
        };

        /* 8723 */
        struct netProfilePacket_t
        {
            int iTime;
            int iSize;
            int bFragment;
        };

        /* 8724 */
        struct netProfileStream_t
        {
            netProfilePacket_t packets[60];
            int iCurrPacket;
            int iBytesPerSecond;
            int iLastBPSCalcTime;
            int iCountedPackets;
            int iCountedFragments;
            int iFragmentPercentage;
            int iLargestPacket;
            int iSmallestPacket;
        };

        /* 8755 */
        struct netProfileInfo_t
        {
            netProfileStream_t send;
            netProfileStream_t recieve;
        };

        /* 8758 */
        struct netchan_t
        {
            int outgoingSequence;
            netsrc_t sock;
            int dropped;
            int incomingSequence;
            netadr_t remoteAddress;
            int fragmentSequence;
            int fragmentLength;
            unsigned __int8 *fragmentBuffer;
            int fragmentBufferSize;
            int unsentFragments;
            int unsentFragmentStart;
            int unsentLength;
            unsigned __int8 *unsentBuffer;
            int unsentBufferSize;
            netProfileInfo_t prof;
        };

        /* 9758 */
        const struct clientHeader_t
        {
            int state;
            int sendAsActive;
            int deltaMessage;
            int rateDelayed;
            netchan_t netchan;
            float predictedOrigin[3];
            int predictedOriginServerTime;
        };

        static_assert(offsetof(clientHeader_t, deltaMessage) == 0x8, "");

        /* 9760 */
        struct svscmd_info_t
        {
            char cmd[1024];
            int time;
            int type;
        };

        struct __declspec(align(4)) client_t
        {
            clientHeader_t header;
            const char *dropReason;
            char userinfo[1024];
            svscmd_info_t reliableCommandInfo[128];
            int reliableSequence;
            int reliableAcknowledge;
            int reliableSent;
            int messageAcknowledge;
            int gamestateMessageNum;
            int challenge;
            usercmd_s lastUsercmd;              // 0x20E5C, 0x0020
            int lastClientCommand;              // 0x20E7C, 0x0004
            char lastClientCommandString[1024]; // 0x20E80, 0x0004
            gentity_s *gentity;                 // 0x21280, 0x0004
            char name[32];                      // 0x21284, 0x0020
            char _padding[0x819e4];             // Padding to reach 666760 bytes
        };

        static_assert(sizeof(client_t) == 666760, "");
        static_assert(offsetof(client_t, gentity) == 0x21280, "");

        struct clientState_s;
        struct svEntity_s;
        struct archivedEntity_s;
        struct cachedClient_s;
        struct cachedSnapshot_t;

        /* 9766 */
        struct serverStaticHeader_t
        {
            client_t *clients;
            int time;
            int snapFlagServerBit;
            int numSnapshotEntities;
            int numSnapshotClients;
            int nextSnapshotEntities;
            entityState_s *snapshotEntities;
            clientState_s *snapshotClients;
            svEntity_s *svEntities;
            float mapCenter[3];
            archivedEntity_s *cachedSnapshotEntities;
            cachedClient_s *cachedSnapshotClients;
            unsigned __int8 *archivedSnapshotBuffer;
            cachedSnapshot_t *cachedSnapshotFrames;
            int nextCachedSnapshotFrames;
            int nextArchivedSnapshotFrames;
            int nextCachedSnapshotEntities;
            int nextCachedSnapshotClients;
            int num_entities;
            int maxclients;
            int fps;
            int clientArchive;
            gentity_s *gentities;
            int gentitySize;
            clientState_s *firstClientState;
            playerState_s *firstPlayerState;
            int clientSize;
            unsigned int pad[3];
        };

        static_assert(sizeof(serverStaticHeader_t) == 0x0080, "");

        enum DvarFlags : unsigned __int16
        {
            DVAR_FLAG_NONE = 0x0,
            DVAR_CODINFO = 0x100, // On change, this is sent to all clients (if you are host)
        };

        union DvarValue
        {
            bool enabled;
            int integer;
            unsigned int unsignedInt;
            float value;
            float vector[4];
            const char *string;
            unsigned __int8 color[4];
        };

        union DvarLimits
        {
            struct
            {
                int stringCount;
                const char **strings;
            } enumeration;

            struct
            {
                int min;
                int max;
            } integer;

            struct
            {
                float min;
                float max;
            } value;

            struct
            {
                float min;
                float max;
            } vector;
        };

        struct dvar_s
        {
            const char *name;
            const char *description;
            unsigned __int16 flags;
            unsigned __int8 type;
            bool modified;
            DvarValue current;
            DvarValue latched;
            DvarValue reset;
            DvarLimits domain;
            dvar_s *hashNext;
        };

        struct ScreenPlacement
        {
            float scaleVirtualToReal[2];
            float scaleVirtualToFull[2];
            float scaleRealToVirtual[2];
            float virtualViewableMin[2];
            float virtualViewableMax[2];
            float realViewportSize[2];
            float realViewableMin[2];
            float realViewableMax[2];
            float subScreenLeft;
        };

        struct GfxViewInfo;

        struct __declspec(align(4)) pmove_t
        {
            playerState_s *ps;
            usercmd_s cmd;
            usercmd_s oldcmd;
            int tracemask;
            int numtouch;
            int touchents[32];
            float mins[3];
            float maxs[3];
            float xyspeed;
            int proneChange;
            float maxSprintTimeMultiplier;
            bool mantleStarted;
            float mantleEndPos[3];
            int mantleDuration;
            int viewChangeTime;
            float viewChange;
            unsigned __int8 handler;
        };
        static_assert(sizeof(pmove_t) == 272, "");

        struct viewState_t;

        struct weaponState_t;

        enum connstate_t
        {
            CA_DISCONNECTED = 0x0,
            CA_CINEMATIC = 0x1,
            CA_LOGO = 0x2,
            CA_CONNECTING = 0x3,
            CA_CHALLENGING = 0x4,
            CA_CONNECTED = 0x5,
            CA_SENDINGSTATS = 0x6,
            CA_LOADING = 0x7,
            CA_PRIMED = 0x8,
            CA_ACTIVE = 0x9,
        };

        struct clientUIActive_t
        {
            bool active;
            bool isRunning;
            bool cgameInitialized;
            bool cgameInitCalled;
            bool mapPreloaded;
            int keyCatchers;
            bool displayHUDWithKeycatchUI;
            connstate_t connectionState;
            int nextScrollTime;
            bool invited;
            int numVoicePacketsSent;
            int numVoicePacketsSentStart;
        };
        static_assert(sizeof(clientUIActive_t) == 36, "");

        /* 8951 */
        struct __declspec(align(4)) cLeaf_t
        {
            unsigned __int16 firstCollAabbIndex;
            unsigned __int16 collAabbCount;
            int brushContents;
            int terrainContents;
            float mins[3];
            float maxs[3];
            int leafBrushNode;
            __int16 cluster;
        };

        /* 8960 */
        struct cmodel_t
        {
            float mins[3];
            float maxs[3];
            float radius;
            cLeaf_t leaf;
        };

        static_assert(sizeof(cmodel_t) == 72, "");

        /* 8761 */
        struct cplane_s
        {
            float normal[3];
            float dist;
            unsigned __int8 type;
            unsigned __int8 signbits;
            unsigned __int8 pad[2];
        };

        /* 8798 */
        struct __declspec(align(2)) cbrushside_t
        {
            cplane_s *plane;
            unsigned int materialNum;
            __int16 firstAdjacentSideOffset;
            unsigned __int8 edgeCount;
        };

#pragma warning(disable : 4324)

        /* 8961 */
        struct __declspec(align(16)) cbrush_t
        {
            float mins[3];
            int contents;
            float maxs[3];
            unsigned int numsides;
            cbrushside_t *sides;
            __int16 axialMaterialNum[2][3];
            unsigned __int8 *baseAdjacentSide;
            __int16 firstAdjacentSideOffsets[2][3];
            unsigned __int8 edgeCount[2][3];
        };

#pragma warning(default : 4324)

        static_assert(sizeof(cbrush_t) == 80, "");

        // stubs
        struct cStaticModelWritable
        {
            unsigned __int16 nextModelInWorldSector;
        };

        struct cStaticModel_s
        {
            cStaticModelWritable writable;
            XModel *xmodel;
            float origin[3];
            float invScaledAxis[3][3];
            float absmin[3];
            float absmax[3];
        };

        struct dmaterial_t;
        struct cNode_t;
        struct cLeaf_t;
        struct cLeafBrushNode_s;
        struct CollisionBorder;
        struct CollisionPartition;
        struct CollisionAabbTree;
        struct DynEntityDef;
        struct DynEntityPose;
        struct DynEntityClient;
        struct DynEntityColl;

        /* 9079 */
        struct clipMap_t
        {
            const char *name;
            int isInUse;
            int planeCount;
            cplane_s *planes;
            unsigned int numStaticModels;
            cStaticModel_s *staticModelList;
            unsigned int numMaterials;
            dmaterial_t *materials;
            unsigned int numBrushSides;
            cbrushside_t *brushsides;
            unsigned int numBrushEdges;
            unsigned __int8 *brushEdges;
            unsigned int numNodes;
            cNode_t *nodes;
            unsigned int numLeafs;
            cLeaf_t *leafs;
            unsigned int leafbrushNodesCount;
            cLeafBrushNode_s *leafbrushNodes;
            unsigned int numLeafBrushes;
            unsigned __int16 *leafbrushes;
            unsigned int numLeafSurfaces;
            unsigned int *leafsurfaces;
            unsigned int vertCount;
            float (*verts)[3];
            int triCount;
            unsigned __int16 *triIndices;
            unsigned __int8 *triEdgeIsWalkable;
            int borderCount;
            CollisionBorder *borders;
            int partitionCount;
            CollisionPartition *partitions;
            int aabbTreeCount;
            CollisionAabbTree *aabbTrees;
            unsigned int numSubModels;
            cmodel_t *cmodels;
            unsigned __int16 numBrushes;
            cbrush_t *brushes;
            int numClusters;
            int clusterBytes;
            unsigned __int8 *visibility;
            int vised;
            MapEnts *mapEnts;
            cbrush_t *box_brush;
            cmodel_t box_model;
            unsigned __int16 dynEntCount[2];
            DynEntityDef *dynEntDefList[2];
            DynEntityPose *dynEntPoseList[2];
            DynEntityClient *dynEntClientList[2];
            DynEntityColl *dynEntCollList[2];
            unsigned int checksum;
        };
        static_assert(offsetof(clipMap_t, isInUse) == 4, "");
        static_assert(offsetof(clipMap_t, numSubModels) == 132, "");
        static_assert(offsetof(clipMap_t, cmodels) == 136, "");
        static_assert(offsetof(clipMap_t, numBrushes) == 140, "");
        static_assert(offsetof(clipMap_t, brushes) == 144, "");

        struct scr_entref_t
        {
            unsigned __int16 entnum;
            unsigned __int16 classnum;
        };

        /* 9735 */
        struct BuiltinFunctionDef
        {
            const char *actionString;
            void (*actionFunc)();
            int type;
        };

        enum DemoType : __int32
        {
            DEMO_TYPE_NONE = 0x0,
            DEMO_TYPE_CLIENT = 0x1,
            DEMO_TYPE_SERVER = 0x2,
        };

        enum CubemapShot : __int32
        {
            CUBEMAPSHOT_NONE = 0x0,
            CUBEMAPSHOT_RIGHT = 0x1,
            CUBEMAPSHOT_LEFT = 0x2,
            CUBEMAPSHOT_BACK = 0x3,
            CUBEMAPSHOT_FRONT = 0x4,
            CUBEMAPSHOT_UP = 0x5,
            CUBEMAPSHOT_DOWN = 0x6,
            CUBEMAPSHOT_COUNT = 0x7,
        };

        struct snapshot_s
        {
            int snapFlags;
            int ping;
            int serverTime;
            playerState_s ps;
            int numEntities;
            int numClients;
            entityState_s entities[512];
            clientState_s clients[24];
            int serverCommandSequence;
        };

        struct clientControllers_t
        {
            float angles[6][3];
            float tag_origin_angles[3];
            float tag_origin_offset[3];
        };

        struct __declspec(align(4)) CEntPlayerInfo
        {
            clientControllers_t *control;
            unsigned __int8 tag[6];
        };

        struct CEntTurretAngles
        {
            float pitch;
            float yaw;
        };

        union $EA216469116406E3318F58110C4A4856
        {
            CEntTurretAngles angles;
            const float *viewAngles;
        };

        struct CEntTurretInfo
        {
            $EA216469116406E3318F58110C4A4856 ___u0;
            float barrelPitch;
            bool playerUsing;
            unsigned __int8 tag_aim;
            unsigned __int8 tag_aim_animated;
            unsigned __int8 tag_flash;
        };

        struct __declspec(align(2)) CEntVehicleInfo
        {
            __int16 pitch;
            __int16 yaw;
            __int16 roll;
            __int16 barrelPitch;
            float barrelRoll;
            __int16 steerYaw;
            float time;
            unsigned __int16 wheelFraction[4];
            unsigned __int8 wheelBoneIndex[4];
            unsigned __int8 tag_body;
            unsigned __int8 tag_turret;
            unsigned __int8 tag_barrel;
        };

        struct FxEffect;

        struct CEntFx
        {
            int triggerTime;
            FxEffect *effect;
        };

        union $5269F2DE9B9B17DB5A39751B474B0C4C
        {
            CEntPlayerInfo player;
            CEntTurretInfo turret;
            CEntVehicleInfo vehicle;
            CEntFx fx;
        };

        struct cpose_t
        {
            unsigned __int16 lightingHandle;
            unsigned __int8 eType;
            unsigned __int8 eTypeUnion;
            unsigned __int8 localClientNum;
            int cullIn;
            unsigned __int8 isRagdoll;
            int ragdollHandle;
            int killcamRagdollHandle;
            int physObjId;
            float origin[3];
            float angles[3];
            $5269F2DE9B9B17DB5A39751B474B0C4C ___u11;
        };

        struct XAnimTree_s;

        struct centity_s
        {
            cpose_t pose;
            LerpEntityState currentState;
            entityState_s nextState;
            bool nextValid;
            bool bMuzzleFlash;
            bool bTrailMade;
            int previousEventSequence;
            int miscTime;
            float lightingOrigin[3];
            XAnimTree_s *tree;
        };

        struct playerEntity_t
        {
            float fLastWeaponPosFrac;
            int bPositionToADS;
            float vPositionLastOrg[3];
            float fLastIdleFactor;
            float vLastMoveOrg[3];
            float vLastMoveAng[3];
        };

        struct GfxDepthOfField
        {
            float viewModelStart;
            float viewModelEnd;
            float nearStart;
            float nearEnd;
            float farStart;
            float farEnd;
            float nearBlur;
            float farBlur;
        };

        struct GfxFilm
        {
            bool enabled;
            float brightness;
            float contrast;
            float desaturation;
            bool invert;
            float tintDark[3];
            float tintLight[3];
        };

        struct GfxGlow
        {
            bool enabled;
            float bloomCutoff;
            float bloomDesaturation;
            float bloomIntensity;
            float radius;
        };

        struct GfxLightDef;

        struct GfxLight
        {
            unsigned __int8 type;
            unsigned __int8 canUseShadowMap;
            unsigned __int8 unused[2];
            float color[3];
            float dir[3];
            float origin[3];
            float radius;
            float cosHalfFovOuter;
            float cosHalfFovInner;
            int exponent;
            unsigned int spotShadowIndex;
            GfxLightDef *def;
        };

        struct GfxViewport
        {
            int x;
            int y;
            int width;
            int height;
        };

        struct refdef_s
        {
            unsigned int x;
            unsigned int y;
            unsigned int width;
            unsigned int height;
            float tanHalfFovX;
            float tanHalfFovY;
            float vieworg[3];
            float viewaxis[3][3];
            float viewOffset[3];
            int time;
            float zNear;
            float blurRadius;
            GfxDepthOfField dof;
            GfxFilm film;
            GfxGlow glow;
            GfxLight primaryLights[255];
            GfxViewport scissorViewport;
            bool useScissorViewport;
            int localClientNum;
        };

        struct score_t
        {
            int client;
            int score;
            int ping;
            int deaths;
            int team;
            int kills;
            int rank;
            int assists;
            Material *hStatusIcon;
            Material *hRankIcon;
        };

        enum InvalidCmdHintType : __int32
        {
            INVALID_CMD_NONE = 0x0,
            INVALID_CMD_NO_AMMO_BULLETS = 0x1,
            INVALID_CMD_NO_AMMO_FRAG_GRENADE = 0x2,
            INVALID_CMD_NO_AMMO_SPECIAL_GRENADE = 0x3,
            INVALID_CMD_NO_AMMO_FLASH_GRENADE = 0x4,
            INVALID_CMD_STAND_BLOCKED = 0x5,
            INVALID_CMD_CROUCH_BLOCKED = 0x6,
            INVALID_CMD_TARGET_TOO_CLOSE = 0x7,
            INVALID_CMD_LOCKON_REQUIRED = 0x8,
            INVALID_CMD_NOT_ENOUGH_CLEARANCE = 0x9,
        };

        struct viewDamage_t
        {
            int time;
            int duration;
            float yaw;
        };

        const struct __declspec(align(4)) shellshock_parms_t
        {
            char pad[0x268];
        };
        static_assert(sizeof(shellshock_parms_t) == 0x268, "");

        struct shellshock_t
        {
            const shellshock_parms_t *parms;
            int startTime;
            int duration;
            int loopEndTime;
            float sensitivity;
            float viewDelta[2];
            int hasSavedScreen;
        };

        struct visionSetVars_t
        {
            bool glowEnable;
            float glowBloomCutoff;
            float glowBloomDesaturation;
            float glowBloomIntensity0;
            float glowBloomIntensity1;
            float glowRadius0;
            float glowRadius1;
            float glowSkyBleedIntensity0;
            float glowSkyBleedIntensity1;
            bool filmEnable;
            float filmBrightness;
            float filmContrast;
            float filmDesaturation;
            bool filmInvert;
            float filmLightTint[3];
            float filmDarkTint[3];
        };
        static_assert(sizeof(visionSetVars_t) == 0x50, "");

        enum visionSetLerpStyle_t : __int32
        {
            VISIONSETLERP_UNDEFINED = 0x0,
            VISIONSETLERP_NONE = 0x1,
            VISIONSETLERP_TO_LINEAR = 0x2,
            VISIONSETLERP_TO_SMOOTH = 0x3,
            VISIONSETLERP_BACKFORTH_LINEAR = 0x4,
            VISIONSETLERP_BACKFORTH_SMOOTH = 0x5,
        };

        struct visionSetLerpData_t
        {
            int timeStart;
            int timeDuration;
            visionSetLerpStyle_t style;
        };

        struct hudElemSoundInfo_t
        {
            int lastPlayedTime;
        };

        struct cgFrameInfo_t
        {
            float aimSpreadScale;
        };

        // TODO: not correct
        struct __declspec(align(8)) cg_s
        {
            int clientNum;
            int localClientNum;
            DemoType demoType;
            CubemapShot cubemapShot;
            int cubemapSize;
            int renderScreen;
            int latestSnapshotNum;
            int latestSnapshotTime;
            snapshot_s *snap;
            snapshot_s *nextSnap;
            snapshot_s activeSnapshots[2];
            float frameInterpolation;
            int frametime;
            int time;
            int oldTime;
            int physicsTime;
            int mapRestart;
            int renderingThirdPerson;
            playerState_s predictedPlayerState;
            centity_s predictedPlayerEntity;
            playerEntity_t playerEntity;
            int predictedErrorTime;
            float predictedError[3];
            float landChange;
            int landTime;
            float heightToCeiling;
            refdef_s refdef;
            float refdefViewAngles[3];
            float lastVieworg[3];
            float swayViewAngles[3];
            float swayAngles[3];
            float swayOffset[3];
            int iEntityLastType[1024];
            XModel *pEntityLastXModel[1024];
            float zoomSensitivity;
            bool isLoading;
            char objectiveText[1024];
            char scriptMainMenu[256];
            int scoresRequestTime;
            int numScores;
            int teamScores[4];
            int teamPings[4];
            int teamPlayers[4];
            score_t scores[24];
            int scoreLimit;
            int scoreFadeTime;
            int scoresTop;
            int scoresOffBottom;
            int scoresBottom;
            int selectedGamerIndex;
            int bannerLines[4];
            unsigned __int64 selectedGamerXuid;
            int drawHud;
            int crosshairClientNum;
            int crosshairClientLastTime;
            int crosshairClientStartTime;
            int identifyClientNum;
            int cursorHintIcon;
            int cursorHintTime;
            int cursorHintFade;
            int cursorHintString;
            int lastClipFlashTime;
            InvalidCmdHintType invalidCmdHintType;
            int invalidCmdHintTime;
            int lastHealthPulseTime;
            int lastHealthLerpDelay;
            int lastHealthClient;
            float lastHealth;
            float healthOverlayFromAlpha;
            float healthOverlayToAlpha;
            int healthOverlayPulseTime;
            int healthOverlayPulseDuration;
            int healthOverlayPulsePhase;
            bool healthOverlayHurt;
            int healthOverlayLastHitTime;
            float healthOverlayOldHealth;
            int healthOverlayPulseIndex;
            int proneBlockedEndTime;
            int lastStance;
            int lastStanceChangeTime;
            int lastStanceFlashTime;
            int voiceTime;
            unsigned int weaponSelect;
            int weaponSelectTime;
            unsigned int weaponLatestPrimaryIdx;
            int prevViewmodelWeapon;
            int equippedOffHand;
            viewDamage_t viewDamage[8];
            int damageTime;
            float damageX;
            float damageY;
            float damageValue;
            float viewFade;
            int weapIdleTime;
            int nomarks;
            int v_dmg_time;
            float v_dmg_pitch;
            float v_dmg_roll;
            float fBobCycle;
            float xyspeed;
            float kickAVel[3];
            float kickAngles[3];
            float offsetAngles[3];
            float gunPitch;
            float gunYaw;
            float gunXOfs;
            float gunYOfs;
            float gunZOfs;
            float vGunOffset[3];
            float vGunSpeed[3];
            float viewModelAxis[4][3];
            float rumbleScale;
            float compassNorthYaw;
            float compassNorth[2];
            Material *compassMapMaterial;
            float compassMapUpperLeft[2];
            float compassMapWorldSize[2];
            int compassFadeTime;
            int healthFadeTime;
            int ammoFadeTime;
            int stanceFadeTime;
            int sprintFadeTime;
            int offhandFadeTime;
            int offhandFlashTime;
            shellshock_t shellshock;
            // cg_s::<unnamed_tag>testShock;
            struct
            {
                int time;
                int duration;
            } testShock;
            int holdBreathTime;
            int holdBreathInTime;
            int holdBreathDelay;
            float holdBreathFrac;
            float radarProgress;
            float selectedLocation[2];
            SprintState sprintStates;
            int adsViewErrorDone;
            int inKillCam;
            // bgs_t bgs;
            struct
            {
                char pad[663508];
            } bgs; // Placeholder for bgs_t
            // cpose_t viewModelPose;
            struct
            {
                char pad[52];
            } viewModelPose;
            visionSetVars_t visionSetPreLoaded[4];
            char visionSetPreLoadedName[4][64];
            visionSetVars_t visionSetFrom[2];
            visionSetVars_t visionSetTo[2];
            visionSetVars_t visionSetCurrent[2];
            visionSetLerpData_t visionSetLerpData[2];
            char visionNameNaked[64];
            char visionNameNight[64];
            int extraButtons;
            int lastActionSlotTime;
            bool playerTeleported;
            int stepViewStart;
            float stepViewChange;
            cgFrameInfo_t lastFrame;
            hudElemSoundInfo_t hudElemSound[32];
            int vehicleFrame;
        };

        static_assert(sizeof(cg_s) == 985704, "");
        static_assert(offsetof(cg_s, clientNum) == 0, "");
        static_assert(offsetof(cg_s, localClientNum) == 4, "");
        static_assert(offsetof(cg_s, demoType) == 8, "");
        static_assert(offsetof(cg_s, renderScreen) == 20, "");
        static_assert(offsetof(cg_s, latestSnapshotTime) == 28, "");
        static_assert(offsetof(cg_s, snap) == 32, "");
        static_assert(offsetof(cg_s, nextSnap) == 36, "");
        static_assert(offsetof(cg_s, frameInterpolation) == 280360, "");
        static_assert(offsetof(cg_s, frametime) == 280364, "");
        static_assert(offsetof(cg_s, time) == 280368, "");
        static_assert(offsetof(cg_s, oldTime) == 280372, "");
        static_assert(offsetof(cg_s, mapRestart) == 280380, "");
        static_assert(offsetof(cg_s, predictedPlayerState) == 280388, "");
        static_assert(offsetof(cg_s, predictedPlayerState.weapon) == 280624, "");
        static_assert(offsetof(cg_s, predictedErrorTime) == 293036, "");
        static_assert(offsetof(cg_s, predictedError) == 293040, "");
        static_assert(offsetof(cg_s, landChange) == 293052, "");
        static_assert(offsetof(cg_s, landTime) == 293056, "");
        static_assert(offsetof(cg_s, refdef) == 293064, "");
        static_assert(offsetof(cg_s, refdefViewAngles) == 309600, "");
        static_assert(offsetof(cg_s, lastVieworg) == 309612, "");
        static_assert(offsetof(cg_s, invalidCmdHintType) == 320248, "");
        static_assert(offsetof(cg_s, invalidCmdHintTime) == 320252, "");
        static_assert(offsetof(cg_s, proneBlockedEndTime) == 320308, "");
        static_assert(offsetof(cg_s, lastStance) == 320312, "");
        static_assert(offsetof(cg_s, lastStanceChangeTime) == 320316, "");
        static_assert(offsetof(cg_s, kickAVel) == 320492, "");
        static_assert(offsetof(cg_s, kickAngles) == 320504, "");
        static_assert(offsetof(cg_s, nomarks) == 320468, "");
        static_assert(offsetof(cg_s, viewModelAxis) == 320572, "");
        static_assert(offsetof(cg_s, ammoFadeTime) == 320664, "");
        static_assert(offsetof(cg_s, offhandFadeTime) == 320676, "");
        static_assert(offsetof(cg_s, shellshock) == 320684, "");
        static_assert(offsetof(cg_s, testShock.time) == 320716, "");
        static_assert(offsetof(cg_s, testShock.duration) == 320720, "");
        static_assert(offsetof(cg_s, inKillCam) == 320776, "");
        static_assert(offsetof(cg_s, isLoading) == 317856, "");
        static_assert(offsetof(cg_s, viewModelPose) == 984288, "");
        // static_assert(offsetof(cg_s, visionSetPreLoaded) == 984336, "");
        // static_assert(offsetof(cg_s, playerTeleported) == 985552, "");
        // static_assert(offsetof(cg_s, vehicleFrame) == 985696, "");

        typedef void (*BuiltinFunction)();
        typedef void (*BuiltinMethod)(scr_entref_t);

        struct pml_t;
        enum fieldtype_t
        {
            F_INT = 0x0,
            F_FLOAT = 0x1,
            F_LSTRING = 0x2,
            F_STRING = 0x3,
            F_VECTOR = 0x4,
            F_ENTITY = 0x5,
            F_ENTHANDLE = 0x6,
            F_VECTORHACK = 0x7,
            F_OBJECT = 0x8,
            F_MODEL = 0x9,
        };

        typedef void (*ClientFieldFunc)(gclient_s *, const struct client_fields_s *);

        struct client_fields_s
        {
            const char *name;
            int ofs;
            fieldtype_t type;
            ClientFieldFunc setter;
            ClientFieldFunc getter;
        };

        struct ent_field_t
        {
            const char *name;
            int ofs;
            fieldtype_t type;
            void (*callback)(gentity_s *, int);
        };

        struct clientConnection_t
        {
            int qport;
            int clientNum;
            int lastPacketSentTime;
            int lastPacketTime;
            netadr_t serverAddress;
            int connectTime;
            int connectPacketCount;
            char serverMessage[256];
            int challenge;
            int checksumFeed;
            int reliableSequence;
            int reliableAcknowledge;
            char reliableCommands[128][1024];
            int serverMessageSequence;
            int serverCommandSequence;
            int lastExecutedServerCommand;
            char serverCommands[128][1024];
            int timeDemoBaseTime;
            netchan_t netchan;
            char netchanOutgoingBuffer[2048];
            char netchanIncomingBuffer[131072];
            netProfileInfo_t OOBProf;
            unsigned __int8 statPacketsToSend;
            int statPacketSendTime[7];
        };
        static_assert(sizeof(clientConnection_t) == 398692, "");

        struct ClientArchiveData
        {
            int serverTime;
            float origin[3];
            float velocity[3];
            int bobCycle;
            int movementDir;
        };
        static_assert(sizeof(ClientArchiveData) == 36, "");
        static_assert(offsetof(ClientArchiveData, serverTime) == 0x0, "");
        static_assert(offsetof(ClientArchiveData, origin) == 0x4, "");
        static_assert(offsetof(ClientArchiveData, velocity) == 0x10, "");
        static_assert(offsetof(ClientArchiveData, bobCycle) == 0x1C, "");
        static_assert(offsetof(ClientArchiveData, movementDir) == 0x20, "");

        struct clSnapshot_t
        {
            int valid;
            int snapFlags;
            int serverTime;
            int messageNum;
            int deltaNum;
            int ping;
            int cmdNum;
            playerState_s ps;
            int numEntities;
            int numClients;
            int parseEntitiesNum;
            int parseClientsNum;
            int serverCommandNum;
        };
        static_assert(sizeof(clSnapshot_t) == 12184, "");

        struct gameState_t
        {
            int stringOffsets[2350];
            char stringData[131072];
            int dataCount;
        };

        enum StanceState : __int32
        {
            CL_STANCE_STAND = 0x0,
            CL_STANCE_CROUCH = 0x1,
            CL_STANCE_PRONE = 0x2,
        };

        struct outPacket_t
        {
            int p_cmdNumber;
            int p_serverTime;
            int p_realtime;
        };

        struct clientActive_t
        {
            bool usingAds;
            int timeoutcount;
            clSnapshot_t snap;
            bool alwaysFalse;
            int serverTime;
            int oldServerTime;
            int oldFrameServerTime;
            int serverTimeDelta;
            int oldSnapServerTime;
            int extrapolatedSnapshot;
            int newSnapshots;
            gameState_t gameState;
            char mapname[64];
            int parseEntitiesNum;
            int parseClientsNum;
            int mouseDx[2];
            int mouseDy[2];
            int mouseIndex;
            bool stanceHeld;
            StanceState stance;
            StanceState stancePosition;
            int stanceTime;
            int cgameUserCmdWeapon;
            int cgameUserCmdOffHandIndex;
            float cgameFOVSensitivityScale;
            float cgameMaxPitchSpeed;
            float cgameMaxYawSpeed;
            float cgameKickAngles[3];
            float cgameOrigin[3];
            float cgameVelocity[3];
            int cgameBobCycle;
            int cgameMovementDir;
            int cgameExtraButtons;
            int cgamePredictedDataServerTime;
            float viewangles[3];
            int serverId;
            int skelTimeStamp;
            volatile int skelMemPos;
            char skelMemory[262144];
            char *skelMemoryStart;
            bool allowedAllocSkel;
            __declspec(align(4)) usercmd_s cmds[128];
            int cmdNumber;
            ClientArchiveData clientArchive[256];
            int clientArchiveIndex;
            outPacket_t outPackets[32];
            clSnapshot_t snapshots[32];
            entityState_s entityBaselines[1024];
            entityState_s parseEntities[2048];
            clientState_s parseClients[2048];
            int corruptedTranslationFile;
            char translationVersion[256];
            float vehicleViewYaw;
            float vehicleViewPitch;
        };
        static_assert(sizeof(clientActive_t) == 1830628, "");
        static_assert(offsetof(clientActive_t, viewangles) == 152880, "");
        static_assert(offsetof(clientActive_t, cmds) == 415056, "");
        static_assert(offsetof(clientActive_t, cmdNumber) == 419152, "");
        static_assert(offsetof(clientActive_t, clientArchiveIndex) == 428372, "");

        struct FxEffectDef;

        struct __declspec(align(8)) animation_s
        {
            char name[64];
            int initialLerp;
            float moveSpeed;
            int duration;
            int nameHash;
            int flags;
            __int64 movetype;
            int noteType;
        };
        static_assert(sizeof(animation_s) == 0x68, "");

        struct lerpFrame_t
        {
            float yawAngle;
            int yawing;
            float pitchAngle;
            int pitching;
            int animationNumber;
            animation_s *animation;
            int animationTime;
            float oldFramePos[3];
            float animSpeedScale;
            int oldFrameSnapshotTime;
        };
        static_assert(sizeof(lerpFrame_t) == 0x30, "");

        struct __declspec(align(4)) clientInfo_t
        {
            int infoValid;
            int nextValid;
            int clientNum;
            char name[32];
            team_t team;
            team_t oldteam;
            int rank;
            int prestige;
            int perks;
            char clanAbbrev[8];
            int score;
            int location;
            int health;
            char model[64];
            char attachModelNames[6][64];
            char attachTagNames[6][64];
            lerpFrame_t legs;
            lerpFrame_t torso;
            float lerpMoveDir;
            float lerpLean;
            float playerAngles[3];
            int leftHandGun;
            int dobjDirty;
            clientControllers_t control;
            unsigned int clientConditions[10][2];
            XAnimTree_s *pXAnimTree;
            int iDObjWeapon;
            unsigned __int8 weaponModel;
            int stanceTransitionTime;
            int turnAnimEndTime;
            char turnAnimType;
            int attachedVehEntNum;
            int attachedVehSlotIndex;
            bool hideWeapon;
            bool usingKnife;
        };
        static_assert(sizeof(clientInfo_t) == 0x4E4, "");

        struct cgs_t
        {
            int viewX;
            int viewY;
            int viewWidth;
            int viewHeight;
            float viewAspect;
            int serverCommandSequence;
            int processedSnapshotNum;
            int localServer;
            char gametype[32];
            char szHostName[256];
            int maxclients;
            int privateClients;
            char mapname[64];
            int gameEndTime;
            int voteTime;
            int voteYes;
            int voteNo;
            char voteString[256];
            int redCrosshair;
            XModel *gameModels[512];
            const FxEffectDef *fxs[100];
            const FxEffectDef *smokeGrenadeFx;
            shellshock_parms_t holdBreathParams;
            char teamChatMsgs[8][271];
            int teamChatMsgTimes[8];
            int teamChatPos;
            int teamLastChatPos;
            float compassWidth;
            float compassHeight;
            float compassY;
            clientInfo_t corpseinfo[8];
        };
        static_assert(sizeof(cgs_t) == 15972, "");

#define CONTENTS_PLAYERCLIP 0x10000

#define PITCH 0
#define YAW 1
#define ROLL 2

    }
}

#pragma warning(default : 4480)
