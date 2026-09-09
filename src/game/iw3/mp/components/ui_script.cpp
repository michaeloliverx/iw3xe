#include "pch.h"
#include "ui_script.h"

namespace iw3
{
namespace mp
{
std::map<std::string, ui_script_handler_t> ui_script::handlers;
Detour ui_script::UI_RunMenuScript_Detour;

ui_script::ui_script()
{
    UI_RunMenuScript_Detour = Detour(UI_RunMenuScript, UI_RunMenuScript_Hook);
    UI_RunMenuScript_Detour.Install();
}

ui_script::~ui_script()
{
    UI_RunMenuScript_Detour.Remove();
    handlers.clear();
}

void ui_script::add(const char *name, ui_script_handler_t callback)
{
    if (!name || !*name || !callback)
        return;

    handlers[name] = callback;
}

bool ui_script::run(int localClientNum, const char **args)
{
    if (!args || !*args)
        return false;

    const char *customArgs = *args;
    char name[1024];
    if (!String_Parse(&customArgs, name, static_cast<int>(sizeof(name))))
        return false;

    for (std::map<std::string, ui_script_handler_t>::const_iterator handler = handlers.begin();
         handler != handlers.end(); ++handler)
    {
        if (I_stricmp(name, handler->first.c_str()) == 0)
        {
            handler->second(localClientNum, &customArgs);
            return true;
        }
    }

    return false;
}

void ui_script::UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript)
{
    if (run(localClientNum, args))
        return;

    UI_RunMenuScript_Detour.GetOriginal<UI_RunMenuScript_t>()(localClientNum, args, actualScript);
}
} // namespace mp
} // namespace iw3
