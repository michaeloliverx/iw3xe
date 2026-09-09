#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class offline_stats : public Module
{
  public:
    offline_stats();
    ~offline_stats();

  private:
    static void LoadStatsScript(int localClientNum, const char **args);
    static void OpenCreateAClassScript(int localClientNum, const char **args);
    static void LiveStorage_ReadStats_Hook(unsigned int controllerIndex);
    static void LiveStorage_UploadStats_Hook(unsigned int controllerIndex);

    static Detour LiveStorage_ReadStats_Detour;
    static Detour LiveStorage_UploadStats_Detour;
};
} // namespace mp
} // namespace iw3
