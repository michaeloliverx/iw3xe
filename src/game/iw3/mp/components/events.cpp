#include "pch.h"
#include "events.h"

#include "assets.h"
#include "cg.h"
#include "cj_tas.h"
#include "clipmap.h"
#include "command.h"
#include "console.h"
#include "gsc.h"
#include "gsc_functions.h"
#include "image_loader.h"
#include "mpsp.h"
#include "mods.h"
#include "pm.h"
#include "sv_bots.h"
#include "ui_feeder.h"

namespace iw3
{
namespace mp
{
namespace
{
typedef void (*EventHandler)();
typedef void (*AssetLinkHandler)(XAsset *asset);

const char *const CODXE_UI_ZONE = "codxe_ui_mp";

const AssetLinkHandler assetLinkHandlers[] = {
    assets::OnAssetLink,
    mpsp::OnAssetLink,
    image_loader::OnAssetLink,
};

const EventHandler cgDrawActiveHandlers[] = {
    cg::OnCGDrawActive,
    cj_tas::OnCGDrawActive,
    clipmap::OnCGDrawActive,
    pm::OnCGDrawActive,
};

const EventHandler cgInitHandlers[] = {
    cj_tas::OnCGInit,
    clipmap::OnCGInit,
};

const EventHandler delayStreamLoadHandlers[] = {
    image_loader::OnDelayStreamLoad,
};

const EventHandler vmShutdownHandlers[] = {
    GSC::OnVMShutdown,
    GSCFunctions::OnVMShutdown,
    sv_bots::OnVMShutdown,
};

const EventHandler dvarInitHandlers[] = {
    cg::OnDvarInit,
    cj_tas::OnDvarInit,
    clipmap::OnDvarInit,
    ModList::OnDvarInit,
    pm::OnDvarInit,
    UIFeeder::OnDvarInit,
};

const EventHandler cmdInitHandlers[] = {
    command::OnCmdInit,
};

const EventHandler uiRefreshHandlers[] = {
    console::OnUIRefresh,
};
} // namespace

void Events::CG_DrawActive_Hook(int localClientNum)
{
    // Call original function first
    CG_DrawActive_Detour.GetOriginal<decltype(&CG_DrawActive_Hook)>()(localClientNum);

    for (size_t i = 0; i < ARRAYSIZE(cgDrawActiveHandlers); ++i)
    {
        cgDrawActiveHandlers[i]();
    }
}

Detour Events::CG_DrawActive_Detour;

void Events::CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    // Call original function first
    CG_Init_Detour.GetOriginal<CG_Init_t>()(localClientNum, serverMessageNum, serverCommandSequence, clientNum);

    for (size_t i = 0; i < ARRAYSIZE(cgInitHandlers); ++i)
    {
        cgInitHandlers[i]();
    }
}

Detour Events::CG_Init_Detour;

void Events::Load_DelayStream_Hook()
{
    Load_DelayStream_Detour.GetOriginal<Load_DelayStream_t>()();

    for (size_t i = 0; i < ARRAYSIZE(delayStreamLoadHandlers); ++i)
    {
        delayStreamLoadHandlers[i]();
    }
}

Detour Events::Load_DelayStream_Detour;

void Events::Scr_ShutdownSystem_Hook(unsigned __int8 sys)
{
    for (size_t i = 0; i < ARRAYSIZE(vmShutdownHandlers); ++i)
    {
        vmShutdownHandlers[i]();
    }

    // Call original function after callbacks
    Scr_ShutdownSystem_Detour.GetOriginal<Scr_ShutdownSystem_t>()(sys);
}

Detour Events::Scr_ShutdownSystem_Detour;

void Events::Com_InitDvars_Hook()
{
    // Let the game initialize its dvar subsystem first, then register plugin dvars before engine init continues.
    Com_InitDvars_Detour.GetOriginal<Com_InitDvars_t>()();

    for (size_t i = 0; i < ARRAYSIZE(dvarInitHandlers); ++i)
    {
        dvarInitHandlers[i]();
    }
}

Detour Events::Com_InitDvars_Detour;

void Events::Cmd_Init_Hook()
{
    // Call original function first so the command subsystem is ready.
    Cmd_Init_Detour.GetOriginal<Cmd_Init_t>()();

    for (size_t i = 0; i < ARRAYSIZE(cmdInitHandlers); ++i)
    {
        cmdInitHandlers[i]();
    }
}

Detour Events::Cmd_Init_Detour;

void Events::UI_Refresh_Hook(int localClientNum)
{
    UI_Refresh_Detour.GetOriginal<decltype(UI_Refresh)>()(localClientNum);

    for (size_t i = 0; i < ARRAYSIZE(uiRefreshHandlers); ++i)
    {
        uiRefreshHandlers[i]();
    }
}

Detour Events::UI_Refresh_Detour;

XAssetEntry *Events::DB_LinkXAssetEntry_Hook(XAsset *asset, int allowOverride)
{
    // During the deferred override pass, asset points at the XAsset member at the start of the pending XAssetEntry.
    // g_zoneIndex may already refer to a later zone, so recover the asset's original zone from the entry itself.
    const unsigned int zoneIndex = allowOverride ? reinterpret_cast<XAssetEntry *>(asset)->zoneIndex : *g_zoneIndex;
    XZoneName *zone = &g_zoneNames[zoneIndex];

    for (size_t i = 0; i < ARRAYSIZE(assetLinkHandlers); ++i)
    {
        assetLinkHandlers[i](asset);
    }

    // patch_mp has a higher database priority than ui_mp. Raise only the link-time priority of our UI zone so its
    // replacement assets win, while preserving the zone's ui_mp allocation flag and native unload lifecycle.
    const int originalZoneFlags = zone->flags;
    if (I_stricmp(zone->name, CODXE_UI_ZONE) == 0)
        zone->flags = DB_ZONE_DEV;

    XAssetEntry *entry = DB_LinkXAssetEntry_Detour.GetOriginal<DB_LinkXAssetEntry_t>()(asset, allowOverride);
    zone->flags = originalZoneFlags;
    return entry;
}

Detour Events::DB_LinkXAssetEntry_Detour;

Events::Events()
{
    CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();

    CG_Init_Detour = Detour(CG_Init, CG_Init_Hook);
    CG_Init_Detour.Install();

    Load_DelayStream_Detour = Detour(Load_DelayStream, Load_DelayStream_Hook);
    Load_DelayStream_Detour.Install();

    Scr_ShutdownSystem_Detour = Detour(Scr_ShutdownSystem, Scr_ShutdownSystem_Hook);
    Scr_ShutdownSystem_Detour.Install();

    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();

    Cmd_Init_Detour = Detour(Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();

    UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
    UI_Refresh_Detour.Install();

    DB_LinkXAssetEntry_Detour = Detour(DB_LinkXAssetEntry, DB_LinkXAssetEntry_Hook);
    DB_LinkXAssetEntry_Detour.Install();
}

Events::~Events()
{
    CG_DrawActive_Detour.Remove();
    CG_Init_Detour.Remove();
    Load_DelayStream_Detour.Remove();
    Scr_ShutdownSystem_Detour.Remove();
    Com_InitDvars_Detour.Remove();
    Cmd_Init_Detour.Remove();
    UI_Refresh_Detour.Remove();
    DB_LinkXAssetEntry_Detour.Remove();
}

} // namespace mp
} // namespace iw3
