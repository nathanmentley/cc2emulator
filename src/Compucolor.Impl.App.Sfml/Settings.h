#pragma once

#include <map>
#include <string>

namespace Compucolor::Impl::App::Sfml
{
    // User-editable application settings persisted as key=value lines.
    class Settings {
        public:
            Settings();

            static std::string DefaultPath();

            // Returns false if the file is missing or unreadable; defaults stay in effect.
            bool Load(const std::string& path);
            bool Save(const std::string& path) const;

            [[nodiscard]] std::string GetString(const std::string& key) const;
            [[nodiscard]] int GetInt(const std::string& key) const;
            [[nodiscard]] bool GetBool(const std::string& key) const;

            void SetString(const std::string& key, const std::string& value);
            void SetInt(const std::string& key, int value);
            void SetBool(const std::string& key, bool value);

        private:
            std::map<std::string, std::string> _data;
    };
}
