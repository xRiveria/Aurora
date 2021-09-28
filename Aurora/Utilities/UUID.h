#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <random>

/* UUID

    - UUID consists of 16 octets - represented as 32 hexadecimal digits, displayed in 5 groups seperated by hyphens.
    - A UUID is represented as 8-4-4-4-12 for a total of 36 characters (32 hexadecimal characters and 4 hyphens).
    - https://en.wikipedia.org/wiki/Universally_unique_identifier#Format
*/

namespace Aurora
{
    class UUID
    {
    public:
        UUID()
        {
            GenerateNewUUID();
        }

        void GenerateNewUUID()
        {
            // 16 Octets (Or 16 Chars), Hexadecimal Represented As 8-4-4-4-12.
            m_UUID = GenerateHex(4) + "-" + GenerateHex(2) + "-" + GenerateHex(2) + "-" + GenerateHex(2) + "-" + GenerateHex(6);
        }

        const std::string GetUUID() const { return m_UUID; }

        bool operator==(const UUID& otherUUID)
        {
            if (m_UUID == otherUUID.GetUUID())
            {
                return true;
            }

            return false;
        }

        bool operator!=(const UUID& otherUUID)
        {
            !(*this == otherUUID);
        }

    private:
        unsigned int GenerateRandomChar()
        {
            std::random_device randomDevice;
            std::mt19937 generator(randomDevice()); // Seed
            std::uniform_int_distribution<> distribution(0, 255);

            return distribution(generator);
        }

        std::string GenerateHex(const unsigned int length)
        {
            std::stringstream stringStream;
            for (auto i = 0; i < length; i++)
            {
                const unsigned int randomChar = GenerateRandomChar();
                std::stringstream hexStream;
                hexStream << std::hex << randomChar;
                std::string hex = hexStream.str();
                stringStream << (hex.length() < 2 ? '0' + hex : hex); // Represent decimals smaller than 16 as a pair of 0(Hex).
            }

            return stringStream.str();
        }

    private:
        std::string m_UUID;
    };
}