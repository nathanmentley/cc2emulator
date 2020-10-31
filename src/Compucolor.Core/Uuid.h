#pragma once

#include <cctype>
#include <random>
#include <sstream>
#include <string>

class Uuid {
    public:
        static Uuid Generate();

        bool operator==(const Uuid& other);
        bool operator!=(const Uuid& other);

        std::string GetRaw();

    private:
        std::string _data;

        Uuid();

        static std::string GenerateUuid();
        static std::string GetHex(const unsigned int len);
        static unsigned int FetchRandomChar();
        static bool CaseInsensitiveEquals(std::string & str1, std::string &str2);
};