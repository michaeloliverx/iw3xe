#pragma once

#include "common.h"

extern const char *CONFIG_PATH;
extern const char *MOD_DIR;
extern const char *DUMP_DIR;

bool DirectoryExists(const char *path);
bool FileExists(const char *path);
bool ReadFileToString(const char *path, std::string &outString);

struct Config
{
    std::string active_mod;
    bool dump_raw;
    bool dump_map_ents;

    Config()
    {
        active_mod = "";
        dump_raw = false;
        dump_map_ents = false;
    }

    std::string GetModBasePath()
    {
        std::string mod_base_path = std::string(MOD_DIR) + "\\" + active_mod;
        if (!DirectoryExists(mod_base_path.c_str()))
        {
            DbgPrint("Config: Active mod directory does not exist: %s\n", mod_base_path.c_str());
            return "";
        }
        return mod_base_path;
    }
};

bool LoadConfigFromJson(const char *jsonBuffer, DWORD bufferSize, Config &outConfig);
bool LoadConfigFromFile(const char *path, Config &outConfig);
