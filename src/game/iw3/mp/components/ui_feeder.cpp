#include "pch.h"
#include "ui_feeder.h"
#include "command.h"

namespace iw3
{
namespace mp
{
namespace
{
const float MODS_FEEDER_ID = 9.0f;
const float USERMAPS_FEEDER_ID = 60.0f;
const char *const USERMAPS_DIRECTORY = "game:\\_codxe\\usermaps";

struct FeederEntry
{
    std::string name;
    std::string displayName;
};

std::vector<FeederEntry> mods;
std::vector<FeederEntry> usermaps;
int selectedMod = 0;
int selectedUsermap = 0;
bool modsScanned = false;
bool usermapsScanned = false;

Detour UI_FeederCount_Detour;
Detour UI_FeederItemText_Detour;
Detour UI_FeederSelection_Detour;

std::string ReadDisplayName(const std::string &descriptionPath, const std::string &fallback)
{
    std::string displayName = filesystem::ReadFileToString(descriptionPath);
    const size_t lineEnd = displayName.find_first_of("\r\n");
    if (lineEnd != std::string::npos)
        displayName.erase(lineEnd);

    size_t first = 0;
    while (first < displayName.length() && static_cast<unsigned char>(displayName[first]) <= ' ')
        ++first;

    size_t last = displayName.length();
    while (last > first && static_cast<unsigned char>(displayName[last - 1]) <= ' ')
        --last;

    if (first == last)
        return fallback;

    return displayName.substr(first, last - first);
}

bool ContainsEntry(const std::vector<FeederEntry> &entries, const std::string &name)
{
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (I_stricmp(entries[i].name.c_str(), name.c_str()) == 0)
            return true;
    }

    return false;
}

void AddEntry(std::vector<FeederEntry> &entries, const std::string &name, const std::string &descriptionPath)
{
    if (name.empty() || ContainsEntry(entries, name))
        return;

    FeederEntry entry;
    entry.name = name;
    entry.displayName = descriptionPath.empty() ? name : ReadDisplayName(descriptionPath, name);
    entries.push_back(entry);
}

void SortEntries(std::vector<FeederEntry> &entries)
{
    std::sort(entries.begin(), entries.end(), [](const FeederEntry &left, const FeederEntry &right) {
        return I_stricmp(left.displayName.c_str(), right.displayName.c_str()) < 0;
    });
}

void ScanMods()
{
    mods.clear();
    selectedMod = 0;
    modsScanned = true;

    const std::string searchPattern = filesystem::JoinPath(MOD_DIR, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DbgPrint("[codxe][IW3][UIFeeder] Mod directory is unavailable: %s\n", MOD_DIR);
        return;
    }

    do
    {
        const std::string filename = findData.cFileName;
        if (filename == "." || filename == ".." || (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

        const std::string directory = filesystem::JoinPath(MOD_DIR, filename.c_str());
        AddEntry(mods, filename, filesystem::JoinPath(directory.c_str(), "description.txt"));
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    SortEntries(mods);

    DbgPrint("[codxe][IW3][UIFeeder] Found %u mod(s) in %s\n", static_cast<unsigned int>(mods.size()), MOD_DIR);
}

void ScanUsermaps()
{
    usermaps.clear();
    selectedUsermap = 0;
    usermapsScanned = true;

    const std::string searchPattern = filesystem::JoinPath(USERMAPS_DIRECTORY, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DbgPrint("[codxe][IW3][UIFeeder] Usermap directory is unavailable: %s\n", USERMAPS_DIRECTORY);
        return;
    }

    do
    {
        const std::string filename = findData.cFileName;
        if (filename == "." || filename == "..")
            continue;

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

        const std::string directory = filesystem::JoinPath(USERMAPS_DIRECTORY, filename.c_str());
        const std::string fastfile = filesystem::JoinPath(directory.c_str(), (filename + ".ff").c_str());
        if (filesystem::FileExists(fastfile.c_str()))
            AddEntry(usermaps, filename, filesystem::JoinPath(directory.c_str(), "description.txt"));
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    SortEntries(usermaps);

    DbgPrint("[codxe][IW3][UIFeeder] Found %u usermap(s) in %s\n", static_cast<unsigned int>(usermaps.size()),
             USERMAPS_DIRECTORY);
}

void EnsureModsScanned()
{
    if (!modsScanned)
        ScanMods();
}

void EnsureUsermapsScanned()
{
    if (!usermapsScanned)
        ScanUsermaps();
}

void ReloadModsCommand()
{
    ScanMods();
}

void ReloadUsermapsCommand()
{
    ScanUsermaps();
}

int UI_FeederCount_Hook(int localClientNum, itemDef_s *item, float feederID)
{
    if (feederID == MODS_FEEDER_ID)
    {
        EnsureModsScanned();
        return static_cast<int>(mods.size());
    }
    else if (feederID == USERMAPS_FEEDER_ID)
    {
        EnsureUsermapsScanned();
        return static_cast<int>(usermaps.size());
    }

    return UI_FeederCount_Detour.GetOriginal<UI_FeederCount_t>()(localClientNum, item, feederID);
}

const char *UI_FeederItemText_Hook(int localClientNum, itemDef_s *item, float feederID, int index,
                                   unsigned int column, Material **handle)
{
    if (feederID != MODS_FEEDER_ID && feederID != USERMAPS_FEEDER_ID)
    {
        return UI_FeederItemText_Detour.GetOriginal<UI_FeederItemText_t>()(localClientNum, item, feederID, index,
                                                                           column, handle);
    }

    if (handle)
        *handle = NULL;

    if (feederID == MODS_FEEDER_ID)
    {
        EnsureModsScanned();
        if (index < 0 || index >= static_cast<int>(mods.size()))
            return "";

        return mods[index].displayName.c_str();
    }

    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(usermaps.size()))
        return "";

    return usermaps[index].displayName.c_str();
}

void UI_FeederSelection_Hook(int localClientNum, float feederID, itemDef_s *item, int index)
{
    if (feederID != MODS_FEEDER_ID && feederID != USERMAPS_FEEDER_ID)
    {
        UI_FeederSelection_Detour.GetOriginal<UI_FeederSelection_t>()(localClientNum, feederID, item, index);
        return;
    }

    if (feederID == MODS_FEEDER_ID)
    {
        EnsureModsScanned();
        if (index < 0 || index >= static_cast<int>(mods.size()))
            return;

        selectedMod = index;
        DbgPrint("[codxe][IW3][UIFeeder] Selected mod: %s\n", mods[selectedMod].name.c_str());
        return;
    }

    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(usermaps.size()))
        return;

    selectedUsermap = index;
    DbgPrint("[codxe][IW3][UIFeeder] Selected usermap: %s\n", usermaps[selectedUsermap].name.c_str());
}
} // namespace

ui_feeder::ui_feeder()
{
    command::add("codxe_reload_mods", ReloadModsCommand);
    command::add("codxe_reload_usermaps", ReloadUsermapsCommand);

    UI_FeederCount_Detour = Detour(UI_FeederCount, UI_FeederCount_Hook);
    UI_FeederCount_Detour.Install();

    UI_FeederItemText_Detour = Detour(UI_FeederItemText, UI_FeederItemText_Hook);
    UI_FeederItemText_Detour.Install();

    UI_FeederSelection_Detour = Detour(UI_FeederSelection, UI_FeederSelection_Hook);
    UI_FeederSelection_Detour.Install();
}

ui_feeder::~ui_feeder()
{
    UI_FeederSelection_Detour.Remove();
    UI_FeederItemText_Detour.Remove();
    UI_FeederCount_Detour.Remove();
}
} // namespace mp
} // namespace iw3
