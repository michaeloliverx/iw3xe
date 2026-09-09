#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class mods : public Module
{
  public:
    mods();

    static void OnDvarInit();

    static const char *GetModsDirectory();
    static std::string GetActiveName();
    static std::string GetActivePath();
    static std::string ResolvePath(const char *relativePath);

    static bool Activate(const char *name);
    static void Clear();

  private:
    static void ReloadCommand();

    static dvar_s *fs_game;
};
} // namespace mp
} // namespace iw3
