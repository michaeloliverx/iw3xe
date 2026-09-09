#include "pch.h"
#include "fastfiles.h"

#include "mods.h"

namespace iw3
{
namespace mp
{
namespace
{
const char *const MOD_ZONE = "mod";
const char *const MOD_FASTFILE = "mod.ff";
const char *const CODXE_COMMON_ZONE = "codxe_common_mp";
const char *const CODXE_UI_ZONE = "codxe_ui_mp";
const char *const CODXE_ZONE_DIRECTORY = "game:\\_codxe\\zone";
const char *const USERMAPS_DIRECTORY = "game:\\_codxe\\usermaps";
const char *const FASTFILE_EXTENSION = ".ff";
const unsigned int MAX_ZONE_COUNT = 16;
const unsigned int CODXE_MENULIST_POOL_SIZE = 256;
const unsigned int CODXE_MENU_POOL_SIZE = 1024;
const unsigned int CODXE_LOCALIZE_POOL_SIZE = 14000;

bool codxeUiPoolsReady = false;

bool IsSafeZoneName(const char *name)
{
    if (!name || !*name || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0)
        return false;

    for (const char *cursor = name; *cursor; ++cursor)
    {
        const unsigned char c = static_cast<unsigned char>(*cursor);
        if (c < ' ' || c == '/' || c == '\\' || c == ':')
            return false;
    }

    return true;
}

bool EndsWith(const std::string &value, const char *suffix)
{
    const size_t suffixLength = std::strlen(suffix);
    return value.length() >= suffixLength &&
           I_stricmp(value.c_str() + value.length() - suffixLength, suffix) == 0;
}

std::string GetCodxeZoneFastfilePath(const char *zoneName)
{
    if (!IsSafeZoneName(zoneName))
        return std::string();

    const std::string filename = std::string(zoneName) + FASTFILE_EXTENSION;
    return filesystem::JoinPath(CODXE_ZONE_DIRECTORY, filename.c_str());
}

bool ContainsZone(const XZoneInfo *zoneInfo, unsigned int zoneCount, const char *name)
{
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name && I_stricmp(zoneInfo[i].name, name) == 0)
            return true;
    }

    return false;
}

const XZoneInfo *FindZone(const XZoneInfo *zoneInfo, unsigned int zoneCount, const char *name)
{
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name && I_stricmp(zoneInfo[i].name, name) == 0)
            return &zoneInfo[i];
    }

    return nullptr;
}

bool IsInitialZoneBatch(const XZoneInfo *zoneInfo, unsigned int zoneCount)
{
    return ContainsZone(zoneInfo, zoneCount, "code_post_gfx_mp") &&
           ContainsZone(zoneInfo, zoneCount, "common_mp");
}

void DisableFastfileAuth()
{
    // DBX_AuthLoad_ValidateHash
    ppc::Nop(0x822B2994);
    ppc::Nop(0x822B2A34);
    ppc::Nop(0x822B2D2C);

    // DBX_AuthLoad_ValidateSignature
    ppc::Nop(0x822B2D44);
}

bool ReallocateUiAssetPools()
{
    if (*g_anyFastFileLoaded)
    {
        DbgPrint("[codxe][IW3][FastFiles] Cannot expand UI asset pools after fastfile loading has started\n");
        return false;
    }

    void *menuListPool = malloc(CODXE_MENULIST_POOL_SIZE * DB_GetXAssetTypeSize(ASSET_TYPE_MENULIST));
    void *menuPool = malloc(CODXE_MENU_POOL_SIZE * DB_GetXAssetTypeSize(ASSET_TYPE_MENU));
    void *localizePool = malloc(CODXE_LOCALIZE_POOL_SIZE * DB_GetXAssetTypeSize(ASSET_TYPE_LOCALIZE_ENTRY));

    if (!menuListPool || !menuPool || !localizePool)
    {
        free(menuListPool);
        free(menuPool);
        free(localizePool);
        DbgPrint("[codxe][IW3][FastFiles] Failed to allocate expanded UI asset pools\n");
        return false;
    }

    DB_XAssetPool[ASSET_TYPE_MENULIST] = menuListPool;
    g_poolSize[ASSET_TYPE_MENULIST] = CODXE_MENULIST_POOL_SIZE;
    DB_XAssetPool[ASSET_TYPE_MENU] = menuPool;
    g_poolSize[ASSET_TYPE_MENU] = CODXE_MENU_POOL_SIZE;
    DB_XAssetPool[ASSET_TYPE_LOCALIZE_ENTRY] = localizePool;
    g_poolSize[ASSET_TYPE_LOCALIZE_ENTRY] = CODXE_LOCALIZE_POOL_SIZE;

    DbgPrint("[codxe][IW3][FastFiles] Expanded UI asset pools: menulist=%u menu=%u localize=%u\n",
             CODXE_MENULIST_POOL_SIZE, CODXE_MENU_POOL_SIZE, CODXE_LOCALIZE_POOL_SIZE);
    return true;
}
} // namespace

Detour fastfiles::DB_BuildOSPath_Detour;
Detour fastfiles::DB_LoadXAssets_Detour;

bool fastfiles::HasModFastfile()
{
    const std::string path = GetModFastfilePath();
    return !path.empty() && filesystem::FileExists(path.c_str());
}

std::string fastfiles::GetModFastfilePath()
{
    return mods::ResolvePath(MOD_FASTFILE);
}

const char *fastfiles::GetUsermapsDirectory()
{
    return USERMAPS_DIRECTORY;
}

std::string fastfiles::GetUsermapFastfilePath(const char *zoneName)
{
    if (!IsSafeZoneName(zoneName) || I_strnicmp(zoneName, "mp_", 3) != 0)
        return std::string();

    std::string filename = zoneName;
    if (EndsWith(filename, ".ff"))
        filename.erase(filename.length() - 3);

    std::string directory = filename;
    if (EndsWith(directory, "_load"))
        directory.erase(directory.length() - 5);

    const std::string usermapDirectory = filesystem::JoinPath(USERMAPS_DIRECTORY, directory.c_str());
    return filesystem::JoinPath(usermapDirectory.c_str(), (filename + ".ff").c_str());
}

int fastfiles::DB_BuildOSPath_Hook(const char *zoneName, unsigned int size, char *filename)
{
    if (zoneName && I_stricmp(zoneName, MOD_ZONE) == 0)
    {
        const std::string path = GetModFastfilePath();
        if (!path.empty())
            return _snprintf_s(filename, size, _TRUNCATE, "%s", path.c_str());
    }

    const std::string usermapPath = GetUsermapFastfilePath(zoneName);
    if (!usermapPath.empty() && filesystem::FileExists(usermapPath.c_str()))
    {
        DbgPrint("[codxe][IW3][FastFiles] Redirecting usermap zone %s to %s\n", zoneName, usermapPath.c_str());
        return _snprintf_s(filename, size, _TRUNCATE, "%s", usermapPath.c_str());
    }

    const std::string codxeZonePath = GetCodxeZoneFastfilePath(zoneName);
    if (!codxeZonePath.empty() && filesystem::FileExists(codxeZonePath.c_str()))
    {
        DbgPrint("[codxe][IW3][FastFiles] Redirecting zone %s to %s\n", zoneName, codxeZonePath.c_str());
        return _snprintf_s(filename, size, _TRUNCATE, "%s", codxeZonePath.c_str());
    }

    return DB_BuildOSPath_Detour.GetOriginal<DB_BuildOSPath_t>()(zoneName, size, filename);
}

void fastfiles::DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    auto original = DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>();
    const std::string codxeCommonFastfile = GetCodxeZoneFastfilePath(CODXE_COMMON_ZONE);
    const std::string codxeUiFastfile = GetCodxeZoneFastfilePath(CODXE_UI_ZONE);
    const XZoneInfo *stockCommonZone = FindZone(zoneInfo, zoneCount, "common_mp");
    const XZoneInfo *stockUiZone = FindZone(zoneInfo, zoneCount, "ui_mp");
    const bool injectCommon = codxeUiPoolsReady && filesystem::FileExists(codxeCommonFastfile.c_str()) &&
                              stockCommonZone && !ContainsZone(zoneInfo, zoneCount, CODXE_COMMON_ZONE);
    const bool injectUi = codxeUiPoolsReady && filesystem::FileExists(codxeUiFastfile.c_str()) && stockUiZone &&
                          !ContainsZone(zoneInfo, zoneCount, CODXE_UI_ZONE);
    const bool injectMod = HasModFastfile() && IsInitialZoneBatch(zoneInfo, zoneCount) &&
                           !ContainsZone(zoneInfo, zoneCount, MOD_ZONE);
    const unsigned int injectionCount = (injectCommon ? 1 : 0) + (injectUi ? 1 : 0) + (injectMod ? 1 : 0);
    const bool hasCapacity = zoneCount <= MAX_ZONE_COUNT && injectionCount <= MAX_ZONE_COUNT - zoneCount;

    if (injectionCount == 0 || !hasCapacity)
    {
        original(zoneInfo, zoneCount, sync);
        return;
    }

    XZoneInfo zones[MAX_ZONE_COUNT];
    unsigned int outputZoneCount = 0;

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        zones[outputZoneCount] = zoneInfo[i];
        ++outputZoneCount;

        if (injectCommon && zoneInfo[i].name && I_stricmp(zoneInfo[i].name, "common_mp") == 0)
        {
            zones[outputZoneCount].name = CODXE_COMMON_ZONE;
            zones[outputZoneCount].allocFlags = stockCommonZone->allocFlags;
            zones[outputZoneCount].freeFlags = stockCommonZone->freeFlags;
            ++outputZoneCount;

            DbgPrint("[codxe][IW3][FastFiles] Loading common fastfile: %s\n", codxeCommonFastfile.c_str());
        }

        if (injectUi && zoneInfo[i].name && I_stricmp(zoneInfo[i].name, "ui_mp") == 0)
        {
            zones[outputZoneCount].name = CODXE_UI_ZONE;
            zones[outputZoneCount].allocFlags = stockUiZone->allocFlags;
            zones[outputZoneCount].freeFlags = stockUiZone->freeFlags;
            ++outputZoneCount;

            DbgPrint("[codxe][IW3][FastFiles] Loading UI fastfile: %s\n", codxeUiFastfile.c_str());
        }
    }

    if (injectMod)
    {
        zones[outputZoneCount].name = MOD_ZONE;
        zones[outputZoneCount].allocFlags = DB_ZONE_MOD;
        zones[outputZoneCount].freeFlags = 0;
        ++outputZoneCount;

        DbgPrint("[codxe][IW3][FastFiles] Loading startup mod fastfile: %s\n", GetModFastfilePath().c_str());
    }

    original(zones, outputZoneCount, sync);
}

void fastfiles::ReloadModZone()
{
    XZoneInfo zones[2];
    unsigned int zoneCount = 1;

    // A null zone name with freeFlags is the engine's native free-only descriptor.
    zones[0].name = nullptr;
    zones[0].allocFlags = DB_ZONE_NONE;
    zones[0].freeFlags = DB_ZONE_MOD;

    const std::string path = GetModFastfilePath();
    if (!path.empty() && filesystem::FileExists(path.c_str()))
    {
        DbgPrint("[codxe][IW3][FastFiles] Loading mod fastfile: %s\n", path.c_str());
        zones[1].name = MOD_ZONE;
        zones[1].allocFlags = DB_ZONE_MOD;
        zones[1].freeFlags = 0;
        zoneCount = 2;
    }
    else
    {
        DbgPrint("[codxe][IW3][FastFiles] Active mod has no mod.ff; using loose files only\n");
    }

    DB_LoadXAssets(zones, zoneCount, 1);
    DbgPrint("[codxe][IW3][FastFiles] Mod zone reload complete\n");
}

fastfiles::fastfiles()
{
    DisableFastfileAuth();

    codxeUiPoolsReady = ReallocateUiAssetPools();

    DB_BuildOSPath_Detour = Detour(DB_BuildOSPath, DB_BuildOSPath_Hook);
    DB_BuildOSPath_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();
}

fastfiles::~fastfiles()
{
    DB_LoadXAssets_Detour.Remove();
    DB_BuildOSPath_Detour.Remove();
}
} // namespace mp
} // namespace iw3
