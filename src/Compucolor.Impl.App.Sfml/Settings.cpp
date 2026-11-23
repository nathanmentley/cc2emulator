#include "Settings.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

using Compucolor::Impl::App::Sfml::Settings;

Settings::Settings():
    _data{
        {"window_scale", "4"},
        {"fullscreen", "false"},
        {"smooth_scaling", "false"},
        {"charset", "standard"},
        {"rom_version", "v8.79"},
        {"speed_regulated", "true"},
        {"disk_a_path", "disks/chomp.ccvf"},
        {"disk_b_path", "disks/life.ccvf"},
        {"last_disk_dir", ""},
        {"state_path", ""},
        {"sound_enabled", "true"},
        {"sound_volume", "50"},
        {"theme", "3"},
        {"crt_effects", "true"},
        {"crt_scanlines", "true"},
        {"crt_fuzz", "true"},
        {"crt_vignette", "true"},
        {"crt_glow", "true"},
        {"crt_scanline_intensity", "6"},
        {"crt_fuzz_intensity", "25"},
        {"crt_vignette_intensity", "12"},
        {"crt_glow_intensity", "63"}
    }
{
}

std::string Settings::DefaultPath()
{
    namespace fs = std::filesystem;

    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg != nullptr && *xdg != '\0')
    {
        return (fs::path(xdg) / "cc2emu" / "settings.config").string();
    }

    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0')
    {
        return (fs::path(home) / ".config" / "cc2emu" / "settings.config").string();
    }

    return "settings.config";
}

bool Settings::Load(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        return false;
    }

    std::string line;

    while (std::getline(file, line))
    {
        const size_t split = line.find('=');

        if (split != std::string::npos && split > 0)
        {
            _data[line.substr(0, split)] = line.substr(split + 1);
        }
    }

    return true;
}

bool Settings::Save(const std::string& path) const
{
    const std::filesystem::path target(path);

    if (target.has_parent_path())
    {
        std::error_code error;
        std::filesystem::create_directories(target.parent_path(), error);
    }

    std::ofstream file(path, std::ios::trunc);

    if (!file.is_open())
    {
        return false;
    }

    for (const auto& [key, value] : _data)
    {
        file << key << '=' << value << '\n';
    }

    return file.good();
}

std::string Settings::GetString(const std::string& key) const
{
    const auto found = _data.find(key);

    return found == _data.end() ? std::string() : found->second;
}

int Settings::GetInt(const std::string& key) const
{
    try
    {
        return std::stoi(GetString(key));
    }
    catch (const std::exception&)
    {
        return 0;
    }
}

bool Settings::GetBool(const std::string& key) const
{
    const std::string value = GetString(key);

    return value == "true" || value == "1";
}

void Settings::SetString(const std::string& key, const std::string& value)
{
    _data[key] = value;
}

void Settings::SetInt(const std::string& key, const int value)
{
    _data[key] = std::to_string(value);
}

void Settings::SetBool(const std::string& key, const bool value)
{
    _data[key] = value ? "true" : "false";
}
