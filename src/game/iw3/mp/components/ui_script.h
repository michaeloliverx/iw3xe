#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
typedef void (*ui_script_handler_t)(int localClientNum, const char **args);

class ui_script : public Module
{
  public:
    ui_script();
    ~ui_script();

    static void add(const char *name, ui_script_handler_t callback);

  private:
    static bool run(int localClientNum, const char **args);
    static void UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript);

    static std::map<std::string, ui_script_handler_t> handlers;
    static Detour UI_RunMenuScript_Detour;
};
} // namespace mp
} // namespace iw3
