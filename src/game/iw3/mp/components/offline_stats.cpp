#include "pch.h"
#include "offline_stats.h"

#include "stats.h"
#include "ui_script.h"

namespace iw3
{
namespace mp
{
namespace
{
const unsigned int OFFLINE_STATS_MAGIC = 0x434F4458;
const unsigned int OFFLINE_STATS_VERSION = 2;
bool IsValidControllerIndex(unsigned int controllerIndex)
{
    return controllerIndex < 4;
}

std::string GetStatsPath(unsigned int controllerIndex)
{
    char filename[64] = {};
    XUID xuid = 0;

    if (XUserGetXUID(controllerIndex, &xuid) == ERROR_SUCCESS && xuid != 0)
    {
        _snprintf_s(filename, ARRAYSIZE(filename), _TRUNCATE, "%016I64X.stat", xuid);
    }
    else
    {
        _snprintf_s(filename, ARRAYSIZE(filename), _TRUNCATE, "controller_%u.stat", controllerIndex);
    }

    return filesystem::JoinPath("game:\\_codxe\\players\\iw3", filename);
}

bool LoadStats(unsigned int controllerIndex)
{
    if (!IsValidControllerIndex(controllerIndex))
        return false;

    const std::string path = GetStatsPath(controllerIndex);
    const std::string contents = filesystem::ReadFileToString(path);
    if (contents.size() != sizeof(OfflineStatsFile))
        return false;

    const OfflineStatsFile *file = reinterpret_cast<const OfflineStatsFile *>(contents.data());
    if (file->magic != OFFLINE_STATS_MAGIC || file->version != OFFLINE_STATS_VERSION ||
        file->payloadSize != sizeof(file->playerStats))
    {
        Com_PrintWarning(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][OfflineStats] Invalid header in %s\n", path.c_str());
        return false;
    }

    const unsigned int storedChecksum = *reinterpret_cast<const unsigned int *>(file->playerStats);
    const unsigned int calculatedChecksum = LiveStorage_ChecksumGamerStats(&file->playerStats[4]);
    if (storedChecksum != calculatedChecksum)
    {
        Com_PrintWarning(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][OfflineStats] Invalid checksum in %s\n", path.c_str());
        return false;
    }

    playerStatNetworkData &stats = controllerStatData[controllerIndex];
    std::memcpy(stats.playerStats, file->playerStats, sizeof(stats.playerStats));
    stats.statsFetched = true;
    stats.statWriteNeeded = false;
    stats.firstTimeRunning = false;

    Com_Printf(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][OfflineStats] Loaded controller %u from %s\n", controllerIndex,
               path.c_str());
    return true;
}

bool SaveStats(unsigned int controllerIndex)
{
    if (!IsValidControllerIndex(controllerIndex))
        return false;

    playerStatNetworkData &stats = controllerStatData[controllerIndex];
    if (!stats.statsFetched)
        return false;

    *reinterpret_cast<unsigned int *>(stats.playerStats) = LiveStorage_ChecksumGamerStats(&stats.playerStats[4]);

    OfflineStatsFile file = {};
    file.magic = OFFLINE_STATS_MAGIC;
    file.version = OFFLINE_STATS_VERSION;
    file.payloadSize = sizeof(file.playerStats);
    std::memcpy(file.playerStats, stats.playerStats, sizeof(file.playerStats));

    const std::string path = GetStatsPath(controllerIndex);
    if (!filesystem::WriteFileToDisk(path.c_str(), reinterpret_cast<const char *>(&file), sizeof(file)))
    {
        Com_PrintError(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][OfflineStats] Failed to save controller %u to %s\n",
                       controllerIndex, path.c_str());
        return false;
    }

    stats.statWriteNeeded = false;
    stats.firstTimeRunning = false;
    Com_Printf(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][OfflineStats] Saved controller %u to %s\n", controllerIndex,
               path.c_str());
    return true;
}

void InitializeStats(unsigned int controllerIndex)
{
    playerStatNetworkData &statData = controllerStatData[controllerIndex];
    statData.statsFetched = true;
    statData.firstTimeRunning = true;
    LiveStorage_ResetStats(controllerIndex);
    stats::UnlockEverything(controllerIndex);
    SaveStats(controllerIndex);
}

void LoadOrInitializeStats(unsigned int controllerIndex)
{
    if (!IsValidControllerIndex(controllerIndex))
        return;

    if (!LoadStats(controllerIndex))
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER,
                   "[codxe][IW3][OfflineStats] No valid local stats for controller %u; creating defaults\n",
                   controllerIndex);
        InitializeStats(controllerIndex);
    }
}

bool IsOfflineGame()
{
    return Dvar_GetBool("systemlink") || Dvar_GetBool("splitscreen");
}
} // namespace

Detour offline_stats::LiveStorage_ReadStats_Detour;
Detour offline_stats::LiveStorage_UploadStats_Detour;

void offline_stats::LoadStatsScript(int localClientNum, const char **args)
{
    (void)args;

    const int controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
    if (!IsValidControllerIndex(controllerIndex))
    {
        Com_PrintError(CON_CHANNEL_DONT_FILTER,
                       "[codxe][IW3][OfflineStats] Invalid controller %i for local client %i\n", controllerIndex,
                       localClientNum);
        return;
    }

    LoadOrInitializeStats(controllerIndex);
}

void offline_stats::OpenCreateAClassScript(int localClientNum, const char **args)
{
    LoadStatsScript(localClientNum, args);

    const int controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
    if (!IsValidControllerIndex(controllerIndex))
        return;

    Cbuf_ExecuteBuffer(localClientNum, controllerIndex, "set ui_cac_ingame 0\n");
    UI_OpenMenu(localClientNum, "live_cac_popup");
}

void offline_stats::LiveStorage_ReadStats_Hook(unsigned int controllerIndex)
{
    if (XUserGetSigninState(controllerIndex) != eXUserSigninState_SignedInToLive)
    {
        LoadOrInitializeStats(controllerIndex);
        return;
    }

    LiveStorage_ReadStats_Detour.GetOriginal<LiveStorage_ReadStats_t>()(controllerIndex);
}

void offline_stats::LiveStorage_UploadStats_Hook(unsigned int controllerIndex)
{
    if (IsOfflineGame())
    {
        SaveStats(controllerIndex);
        return;
    }

    LiveStorage_UploadStats_Detour.GetOriginal<LiveStorage_UploadStats_t>()(controllerIndex);
}

offline_stats::offline_stats()
{
    UIScript::Add("LoadOfflineStats", LoadStatsScript);
    UIScript::Add("OpenOfflineCreateAClass", OpenCreateAClassScript);

    LiveStorage_ReadStats_Detour = Detour(LiveStorage_ReadStats, LiveStorage_ReadStats_Hook);
    LiveStorage_ReadStats_Detour.Install();

    LiveStorage_UploadStats_Detour = Detour(LiveStorage_UploadStats, LiveStorage_UploadStats_Hook);
    LiveStorage_UploadStats_Detour.Install();
}

offline_stats::~offline_stats()
{
    LiveStorage_ReadStats_Detour.Remove();
    LiveStorage_UploadStats_Detour.Remove();
}
} // namespace mp
} // namespace iw3
