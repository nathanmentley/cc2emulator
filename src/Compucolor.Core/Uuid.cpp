#include <Compucolor.Core/Uuid.h>

Uuid Uuid::Generate()
{
    return Uuid();
}

bool Uuid::operator==(const Uuid& other)
{
    return CaseInsensitiveEquals(
        ((*this)->GetRaw()),
        ((*other).GetRaw())
    );
}

bool Uuid::operator!=(const Uuid& other)
{
    return !(*this == other);
}

std::string Uuid::GetRaw()
{
    return _data;
}

Uuid::Uuid():
    _data(GenerateUuid())
{
}

std::string Uuid::GenerateUuid()
{
    std::stringstream ss;

    ss << GetHex(8);
    ss << "-";
    ss << GetHex(4);
    ss << "-";
    ss << GetHex(4);
    ss << "-";
    ss << GetHex(4);
    ss << "-";
    ss << GetHex(12);

    return ss.str();
}

std::string Uuid::GetHex(const unsigned int len) {
    std::stringstream ss;

    for (unsigned int i = 0; i < len; i++) {
        const auto rc = FetchRandomChar();

        std::stringstream hexstream;
        hexstream << std::hex << rc;

        std::string hex = hexstream.str();

        ss << (hex.length() < 2 ? '0' + hex : hex);
    }

    return ss.str();
}

unsigned int Uuid::FetchRandomChar() {
    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(0, 255);

    return dis(gen);
}

bool Uuid::CaseInsensitiveEquals(std::string & str1, std::string &str2)
{
    return (
        (str1.size() == str2.size()) &&
        std::equal(
            str1.begin(),
            str1.end(),
            str2.begin(),
            [](char & c1, char & c2){
                return (
                    c1 == c2 ||
                    std::toupper(c1) == std::toupper(c2)
                );
            }
        )
    );
}