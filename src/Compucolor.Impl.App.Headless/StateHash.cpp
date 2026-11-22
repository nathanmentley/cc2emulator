#include <Compucolor/App/Replay/StateHash.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
    constexpr uint32_t RotateRight(const uint32_t value, const int bits)
    {
        return (value >> bits) | (value << (32 - bits));
    }

    constexpr std::array<uint32_t, 64> K{
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
        0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
        0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
        0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
        0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
        0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
        0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
    };

    class Sha256Digest {
        public:
            void Update(const uint8_t* data, const std::size_t length)
            {
                _totalBits += static_cast<uint64_t>(length) * 8U;

                std::size_t offset = 0;

                while (offset < length)
                {
                    const std::size_t available = _buffer.size() - _bufferLength;
                    const std::size_t copied = std::min(available, length - offset);

                    std::copy_n(data + offset, copied, _buffer.begin() + _bufferLength);

                    _bufferLength += copied;
                    offset += copied;

                    if (_bufferLength == _buffer.size())
                    {
                        Transform(_buffer.data());
                        _bufferLength = 0;
                    }
                }
            }

            [[nodiscard]] std::string FinalHex()
            {
                const uint64_t bitLength = _totalBits;

                _buffer[_bufferLength++] = 0x80U;

                if (_bufferLength > 56)
                {
                    std::fill(_buffer.begin() + _bufferLength, _buffer.end(), 0);
                    Transform(_buffer.data());
                    _bufferLength = 0;
                }

                std::fill(_buffer.begin() + _bufferLength, _buffer.begin() + 56, 0);

                for (int i = 0; i < 8; i++)
                {
                    _buffer[56 + i] = static_cast<uint8_t>(
                        bitLength >> (56 - (i * 8))
                    );
                }

                Transform(_buffer.data());

                std::ostringstream stream;
                stream << std::hex << std::setfill('0');

                for (const uint32_t word : _hash)
                {
                    stream << std::setw(8) << word;
                }

                return stream.str();
            }

        private:
            std::array<uint32_t, 8> _hash{
                0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
                0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U
            };
            std::array<uint8_t, 64> _buffer{};
            std::size_t _bufferLength = 0;
            uint64_t _totalBits = 0;

            void Transform(const uint8_t* block)
            {
                uint32_t words[64]{};

                for (int i = 0; i < 16; i++)
                {
                    words[i] =
                        (static_cast<uint32_t>(block[i * 4]) << 24U) |
                        (static_cast<uint32_t>(block[(i * 4) + 1]) << 16U) |
                        (static_cast<uint32_t>(block[(i * 4) + 2]) << 8U) |
                        static_cast<uint32_t>(block[(i * 4) + 3]);
                }

                for (int i = 16; i < 64; i++)
                {
                    const uint32_t s0 =
                        RotateRight(words[i - 15], 7) ^
                        RotateRight(words[i - 15], 18) ^
                        (words[i - 15] >> 3U);

                    const uint32_t s1 =
                        RotateRight(words[i - 2], 17) ^
                        RotateRight(words[i - 2], 19) ^
                        (words[i - 2] >> 10U);

                    words[i] = words[i - 16] + s0 + words[i - 7] + s1;
                }

                uint32_t a = _hash[0];
                uint32_t b = _hash[1];
                uint32_t c = _hash[2];
                uint32_t d = _hash[3];
                uint32_t e = _hash[4];
                uint32_t f = _hash[5];
                uint32_t g = _hash[6];
                uint32_t h = _hash[7];

                for (int i = 0; i < 64; i++)
                {
                    const uint32_t s1 =
                        RotateRight(e, 6) ^ RotateRight(e, 11) ^ RotateRight(e, 25);
                    const uint32_t choose = (e & f) ^ ((~e) & g);
                    const uint32_t temp1 =
                        h + s1 + choose + K[static_cast<std::size_t>(i)] + words[i];
                    const uint32_t s0 =
                        RotateRight(a, 2) ^ RotateRight(a, 13) ^ RotateRight(a, 22);
                    const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
                    const uint32_t temp2 = s0 + majority;

                    h = g;
                    g = f;
                    f = e;
                    e = d + temp1;
                    d = c;
                    c = b;
                    b = a;
                    a = temp1 + temp2;
                }

                _hash[0] += a;
                _hash[1] += b;
                _hash[2] += c;
                _hash[3] += d;
                _hash[4] += e;
                _hash[5] += f;
                _hash[6] += g;
                _hash[7] += h;
            }
    };
}

boost::json::value Compucolor::Impl::App::Replay::StateHash::Canonicalize(
    const boost::json::value& value
)
{
    if (value.is_object())
    {
        std::vector<std::pair<std::string, boost::json::value>> members;
        members.reserve(value.as_object().size());

        for (const auto& member : value.as_object())
        {
            members.emplace_back(
                std::string(member.key()),
                Canonicalize(member.value())
            );
        }

        std::sort(
            members.begin(),
            members.end(),
            [](const auto& left, const auto& right) {
                return left.first < right.first;
            }
        );

        boost::json::object result;

        for (auto& [key, memberValue] : members)
        {
            result[key] = std::move(memberValue);
        }

        return boost::json::value(std::move(result));
    }

    if (value.is_array())
    {
        boost::json::array result;
        result.reserve(value.as_array().size());

        for (const auto& item : value.as_array())
        {
            result.push_back(Canonicalize(item));
        }

        return boost::json::value(std::move(result));
    }

    return value;
}

std::string Compucolor::Impl::App::Replay::StateHash::Sha256(const std::string_view input)
{
    Sha256Digest digest;
    digest.Update(
        reinterpret_cast<const uint8_t*>(input.data()),
        input.size()
    );

    return digest.FinalHex();
}

std::string Compucolor::Impl::App::Replay::StateHash::OfState(const boost::json::value& state)
{
    return Sha256(boost::json::serialize(Canonicalize(state)));
}
