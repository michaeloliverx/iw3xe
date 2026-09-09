#include "pch.h"
#include "mods.h"

#include "command.h"
#include "fastfiles.h"

namespace iw3
{
namespace mp
{
namespace
{
const char *const MODS_DIRECTORY = "game:\\_codxe\\mods";
const char *const FS_GAME_PREFIX = "mods/";

bool IsSafeModName(const char *name)
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

bool DirectoryExists(const std::string &path)
{
    const DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != static_cast<DWORD>(-1) && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

std::string BuildFsGame(const char *name)
{
    return std::string(FS_GAME_PREFIX) + name;
}
} // namespace

dvar_s *mods::fs_game = nullptr;

mods::mods()
{
    command::add("codxe_reload_mod", ReloadCommand);
}

void mods::OnDvarInit()
{
    fs_game = Dvar_RegisterString("fs_game", "", DVAR_ARCHIVE, "The active mod directory");
}

const char *mods::GetModsDirectory()
{
    return MODS_DIRECTORY;
}

std::string mods::GetActiveName()
{
    if (!fs_game || !fs_game->current.string)
        return std::string();

    const char *value = fs_game->current.string;
    const size_t prefixLength = std::strlen(FS_GAME_PREFIX);
    if (I_strnicmp(value, FS_GAME_PREFIX, static_cast<int>(prefixLength)) != 0)
        return std::string();

    const char *name = value + prefixLength;
    return IsSafeModName(name) ? name : std::string();
}

std::string mods::GetActivePath()
{
    const std::string name = GetActiveName();
    return name.empty() ? std::string() : filesystem::JoinPath(MODS_DIRECTORY, name.c_str());
}

std::string mods::ResolvePath(const char *relativePath)
{
    const std::string activePath = GetActivePath();
    if (activePath.empty() || !relativePath || !*relativePath)
        return std::string();

    return filesystem::JoinPath(activePath.c_str(), relativePath);
}

bool mods::Activate(const char *name)
{
    if (!fs_game || !IsSafeModName(name))
        return false;

    const std::string modPath = filesystem::JoinPath(MODS_DIRECTORY, name);
    if (!DirectoryExists(modPath))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Mod directory does not exist: %s\n", modPath.c_str());
        return false;
    }

    const std::string fsGame = BuildFsGame(name);
    DbgPrint("[codxe][IW3][Mods] Activating %s\n", fsGame.c_str());
    Dvar_SetStringFromSource(fs_game, fsGame.c_str(), DVAR_SOURCE_INTERNAL);
    fastfiles::ReloadModZone();
    Cbuf_AddText(0, "exec mod.cfg\n");
    return true;
}

void mods::Clear()
{
    if (!fs_game)
        return;

    DbgPrint("[codxe][IW3][Mods] Clearing %s\n",
             fs_game->current.string && *fs_game->current.string ? fs_game->current.string : "<none>");
    Dvar_SetStringFromSource(fs_game, "", DVAR_SOURCE_INTERNAL);
    fastfiles::ReloadModZone();
}

void mods::ReloadCommand()
{
    fastfiles::ReloadModZone();
    if (!GetActiveName().empty())
        Cbuf_AddText(0, "exec mod.cfg\n");
}

} // namespace mp
} // namespace iw3
