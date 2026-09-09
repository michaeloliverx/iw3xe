#include "pch.h"
#include "ui_script.h"

namespace iw3
{
namespace mp
{
std::map<std::string, UIScriptHandler_t> UIScript::Scripts;
Detour UIScript::UI_RunMenuScript_Detour;

UIScript::UIScript()
{
    UI_RunMenuScript_Detour = Detour(UI_RunMenuScript, UI_RunMenuScript_Hook);
    UI_RunMenuScript_Detour.Install();
}

UIScript::~UIScript()
{
    UI_RunMenuScript_Detour.Remove();
    Scripts.clear();
}

void UIScript::Add(const char *name, UIScriptHandler_t callback)
{
    if (!name || !*name || !callback)
        return;

    Scripts[name] = callback;
}

bool UIScript::Run(int localClientNum, const char **args)
{
    if (!args || !*args)
        return false;

    const char *customArgs = *args;
    char name[1024];
    if (!String_Parse(&customArgs, name, static_cast<int>(sizeof(name))))
        return false;

    for (std::map<std::string, UIScriptHandler_t>::const_iterator script = Scripts.begin(); script != Scripts.end();
         ++script)
    {
        if (I_stricmp(name, script->first.c_str()) == 0)
        {
            script->second(localClientNum, &customArgs);
            return true;
        }
    }

    return false;
}

void UIScript::UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript)
{
    if (Run(localClientNum, args))
        return;

    UI_RunMenuScript_Detour.GetOriginal<UI_RunMenuScript_t>()(localClientNum, args, actualScript);
}
} // namespace mp
} // namespace iw3
