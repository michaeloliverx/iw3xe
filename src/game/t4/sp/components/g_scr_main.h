#pragma once

#include "common.h"

namespace t4
{
    namespace sp
    {
        // Standalone functions for adding script methods and functions
        void Scr_AddMethod(const char *name, BuiltinMethod func, int type);
        void Scr_AddFunction(const char *name, BuiltinFunction func, int type);

        class g_scr_main : public Module
        {
        public:
            g_scr_main();
            ~g_scr_main();
            const char *get_name() override { return "g_scr_main"; };
        };
    }
}
