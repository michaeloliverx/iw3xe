#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
typedef void (*UIScriptHandler_t)(int localClientNum, const char **args);

class UIScript : public Module
{
  public:
    UIScript();
    ~UIScript();

    static void Add(const char *name, UIScriptHandler_t callback);

  private:
    static bool Run(int localClientNum, const char **args);
    static void UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript);

    static std::map<std::string, UIScriptHandler_t> Scripts;
    static Detour UI_RunMenuScript_Detour;
};
} // namespace mp
} // namespace iw3
