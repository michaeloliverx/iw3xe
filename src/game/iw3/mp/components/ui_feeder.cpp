#include "pch.h"
#include "ui_feeder.h"
#include "fastfiles.h"
#include "mods.h"
#include "ui_script.h"

namespace iw3
{
namespace mp
{
namespace
{
const float MODS_FEEDER_ID = 9.0f;
const float USERMAPS_FEEDER_ID = 60.0f;
const unsigned int FEEDER_BACKGROUND_COLUMN = 0;
const unsigned int FEEDER_BACKGROUND_END_COLUMN = 1;
const unsigned int FEEDER_NAME_COLUMN = 2;
const unsigned int FEEDER_ACTION_COLUMN = 3;
const char *const FEEDER_ACTION_GLYPH = "\x01";
const char *const FEEDER_BACKGROUND_MATERIAL = "gradient_fadein";
const char *const FEEDER_BACKGROUND_END_MATERIAL = "button_highlight_end";

std::vector<FeederEntry> modEntries;
std::vector<FeederEntry> usermaps;
int selectedMod = 0;
int selectedUsermap = 0;
bool modsScanned = false;
bool usermapsScanned = false;
dvar_s *uiCodxeUsermapCounter = NULL;

Detour UI_FeederCount_Detour;
Detour UI_FeederItemColor_Detour;
Detour UI_FeederItemText_Detour;
Detour UI_FeederSelection_Detour;
Detour Item_ListBox_Scroll_Detour;

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

void UpdateUsermapCounter()
{
    if (!uiCodxeUsermapCounter)
        return;

    char counter[32] = "";
    if (!usermaps.empty())
    {
        _snprintf_s(counter, ARRAYSIZE(counter), _TRUNCATE, "%u / %u",
                    static_cast<unsigned int>(selectedUsermap + 1), static_cast<unsigned int>(usermaps.size()));
    }

    Dvar_SetStringFromSource(uiCodxeUsermapCounter, counter, DVAR_SOURCE_INTERNAL);
}

void ScanMods()
{
    modEntries.clear();
    selectedMod = 0;
    modsScanned = true;

    const char *modsDirectory = mods::GetModsDirectory();
    const std::string searchPattern = filesystem::JoinPath(modsDirectory, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DbgPrint("[codxe][IW3][UIFeeder] Mod directory is unavailable: %s\n", modsDirectory);
        return;
    }

    do
    {
        const std::string filename = findData.cFileName;
        if (filename == "." || filename == ".." || (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

        const std::string directory = filesystem::JoinPath(modsDirectory, filename.c_str());
        AddEntry(modEntries, filename, filesystem::JoinPath(directory.c_str(), "description.txt"));
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    SortEntries(modEntries);

    DbgPrint("[codxe][IW3][UIFeeder] Found %u mod(s) in %s\n", static_cast<unsigned int>(modEntries.size()),
             modsDirectory);
}

void ScanUsermaps()
{
    usermaps.clear();
    selectedUsermap = 0;
    usermapsScanned = true;

    const char *usermapsDirectory = fastfiles::GetUsermapsDirectory();
    const std::string searchPattern = filesystem::JoinPath(usermapsDirectory, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        UpdateUsermapCounter();
        DbgPrint("[codxe][IW3][UIFeeder] Usermap directory is unavailable: %s\n", usermapsDirectory);
        return;
    }

    do
    {
        const std::string filename = findData.cFileName;
        if (filename == "." || filename == "..")
            continue;

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

        const std::string directory = filesystem::JoinPath(usermapsDirectory, filename.c_str());
        const std::string fastfile = fastfiles::GetUsermapFastfilePath(filename.c_str());
        if (filesystem::FileExists(fastfile.c_str()))
            AddEntry(usermaps, filename, filesystem::JoinPath(directory.c_str(), "description.txt"));
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    SortEntries(usermaps);
    UpdateUsermapCounter();

    DbgPrint("[codxe][IW3][UIFeeder] Found %u usermap(s) in %s\n", static_cast<unsigned int>(usermaps.size()),
             usermapsDirectory);
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

void LoadModsScript(int /*localClientNum*/, const char ** /*args*/)
{
    ScanMods();
}

void ApplyInitialMapScript(int /*localClientNum*/, const char ** /*args*/)
{
    ScanUsermaps();
}

void RunModScript(int /*localClientNum*/, const char ** /*args*/)
{
    EnsureModsScanned();
    if (selectedMod < 0 || selectedMod >= static_cast<int>(modEntries.size()))
        return;

    mods::Activate(modEntries[selectedMod].name.c_str());
}

void ClearModsScript(int /*localClientNum*/, const char ** /*args*/)
{
    mods::Clear();
}

void ApplyMapScript(int /*localClientNum*/, const char ** /*args*/)
{
    EnsureUsermapsScanned();
    if (selectedUsermap < 0 || selectedUsermap >= static_cast<int>(usermaps.size()))
        return;

    dvar_s *uiMapname = Dvar_FindMalleableVar("ui_mapname");
    if (!uiMapname)
    {
        DbgPrint("[codxe][IW3][UIFeeder] Could not find ui_mapname\n");
        return;
    }

    const FeederEntry &usermap = usermaps[selectedUsermap];
    Dvar_SetStringFromSource(uiMapname, usermap.name.c_str(), DVAR_SOURCE_INTERNAL);
    Party_SetDisplayMapName(usermap.name.c_str());

    dvar_s *uiMapnameText = Dvar_FindMalleableVar("ui_mapname_text");
    if (!uiMapnameText)
        uiMapnameText = Dvar_RegisterString("ui_mapname_text", "", DVAR_FLAG_NONE, "The selected map display name");

    if (uiMapnameText)
        Dvar_SetStringFromSource(uiMapnameText, usermap.displayName.c_str(), DVAR_SOURCE_INTERNAL);

    DbgPrint("[codxe][IW3][UIFeeder] Set selected usermap: %s\n", usermap.name.c_str());
}

int UI_FeederCount_Hook(int localClientNum, itemDef_s *item, float feederID)
{
    if (feederID == MODS_FEEDER_ID)
    {
        EnsureModsScanned();
        return static_cast<int>(modEntries.size());
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

    if (column == FEEDER_BACKGROUND_COLUMN || column == FEEDER_BACKGROUND_END_COLUMN)
    {
        const char *materialName =
            column == FEEDER_BACKGROUND_COLUMN ? FEEDER_BACKGROUND_MATERIAL : FEEDER_BACKGROUND_END_MATERIAL;

        if (handle)
            *handle = DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, materialName).material;

        return "";
    }

    if (feederID == MODS_FEEDER_ID)
    {
        EnsureModsScanned();
        if (index < 0 || index >= static_cast<int>(modEntries.size()))
            return "";

        if (column == FEEDER_ACTION_COLUMN)
            return index == selectedMod ? FEEDER_ACTION_GLYPH : "";

        if (column != FEEDER_NAME_COLUMN)
            return "";

        return modEntries[index].displayName.c_str();
    }

    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(usermaps.size()))
        return "";

    if (column == FEEDER_ACTION_COLUMN)
        return index == selectedUsermap ? FEEDER_ACTION_GLYPH : "";

    if (column != FEEDER_NAME_COLUMN)
        return "";

    return usermaps[index].displayName.c_str();
}

void UI_FeederItemColor_Hook(int localClientNum, itemDef_s *item, float feederID, int index, int column, float *color)
{
    if (feederID == MODS_FEEDER_ID || feederID == USERMAPS_FEEDER_ID)
    {
        if (column == FEEDER_BACKGROUND_COLUMN || column == FEEDER_BACKGROUND_END_COLUMN)
        {
            const bool selected = feederID == MODS_FEEDER_ID ? index == selectedMod : index == selectedUsermap;
            color[0] = 0.9f;
            color[1] = selected ? 0.95f : 0.9f;
            color[2] = 1.0f;
            color[3] = selected ? 0.25f : 0.07f;
        }
        else
        {
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 0.9f;
        }

        return;
    }

    UI_FeederItemColor_Detour.GetOriginal<UI_FeederItemColor_t>()(localClientNum, item, feederID, index, column,
                                                                  color);
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
        if (index < 0 || index >= static_cast<int>(modEntries.size()))
            return;

        selectedMod = index;
        DbgPrint("[codxe][IW3][UIFeeder] Selected mod: %s\n", modEntries[selectedMod].name.c_str());
        return;
    }

    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(usermaps.size()))
        return;

    selectedUsermap = index;
    UpdateUsermapCounter();
    DbgPrint("[codxe][IW3][UIFeeder] Selected usermap: %s\n", usermaps[selectedUsermap].name.c_str());
}

void Item_ListBox_Scroll_Hook(int localClientNum, itemDef_s *item, int max, int scrollMax, int viewMax, int delta)
{
    if (item && item->special == USERMAPS_FEEDER_ID && localClientNum >= 0 && localClientNum < 4 && max > 0)
    {
        if (delta < 0 && item->cursorPos[localClientNum] == 0)
        {
            Item_ListBox_SetCursorPos(localClientNum, item, viewMax, max);
            return;
        }

        if (delta > 0 && item->cursorPos[localClientNum] == max)
        {
            Item_ListBox_SetCursorPos(localClientNum, item, viewMax, 0);
            return;
        }
    }

    Item_ListBox_Scroll_Detour.GetOriginal<Item_ListBox_Scroll_t>()(localClientNum, item, max, scrollMax, viewMax,
                                                                    delta);
}
} // namespace

void ui_feeder::OnDvarInit()
{
    uiCodxeUsermapCounter =
        Dvar_RegisterString("ui_codxe_usermap_counter", "", DVAR_FLAG_NONE, "The selected custom map index");
}

ui_feeder::ui_feeder()
{
    ui_script::add("LoadMods", LoadModsScript);
    ui_script::add("RunMod", RunModScript);
    ui_script::add("ClearMods", ClearModsScript);
    ui_script::add("ApplyInitialMap", ApplyInitialMapScript);
    ui_script::add("ApplyMap", ApplyMapScript);

    UI_FeederCount_Detour = Detour(UI_FeederCount, UI_FeederCount_Hook);
    UI_FeederCount_Detour.Install();

    UI_FeederItemColor_Detour = Detour(UI_FeederItemColor, UI_FeederItemColor_Hook);
    UI_FeederItemColor_Detour.Install();

    UI_FeederItemText_Detour = Detour(UI_FeederItemText, UI_FeederItemText_Hook);
    UI_FeederItemText_Detour.Install();

    UI_FeederSelection_Detour = Detour(UI_FeederSelection, UI_FeederSelection_Hook);
    UI_FeederSelection_Detour.Install();

    Item_ListBox_Scroll_Detour = Detour(Item_ListBox_Scroll, Item_ListBox_Scroll_Hook);
    Item_ListBox_Scroll_Detour.Install();
}

ui_feeder::~ui_feeder()
{
    Item_ListBox_Scroll_Detour.Remove();
    UI_FeederSelection_Detour.Remove();
    UI_FeederItemText_Detour.Remove();
    UI_FeederItemColor_Detour.Remove();
    UI_FeederCount_Detour.Remove();
}
} // namespace mp
} // namespace iw3
