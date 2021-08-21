#pragma once
#include <vector>
#include <fstream>
#include <DirectXMath.h>

using namespace DirectX;

namespace Aurora
{
    class Entity; 

    enum SerializerFlag : uint32_t
    {
        SerializerMode_Read = 1 << 0,
        SerializerMode_Write = 1 << 1,
        SerializerMode_Append = 1 << 2
    };

    class BinarySerializer
    {
    public:
        BinarySerializer(const std::string& filePath, uint32_t serializerFlags);
        ~BinarySerializer();

        bool IsStreamOpen() const { return m_IsStreamOpen; }
        void CloseStream();

        // Writing
        template <class T, class = typename std::enable_if <
            std::is_same<T, bool>::value ||
            std::is_same<T, unsigned char>::value ||
            std::is_same<T, int>::value ||
            std::is_same<T, long>::value ||
            std::is_same<T, long long>::value ||
            std::is_same<T, uint8_t>::value ||
            std::is_same<T, uint16_t>::value ||
            std::is_same<T, uint32_t>::value ||
            std::is_same<T, uint64_t>::value ||
            std::is_same<T, unsigned long>::value ||
            std::is_same<T, unsigned long long>::value ||
            std::is_same<T, float>::value ||
            std::is_same<T, double>::value ||
            std::is_same<T, long double>::value ||
            std::is_same<T, std::byte>::value ||
            std::is_same<T, XMFLOAT2>::value ||
            std::is_same<T, XMFLOAT3>::value>::type>
        void Write(T value)
        {
            m_OutputStream.write(reinterpret_cast<char*>(&value), sizeof(value));
        }

        void Write(const std::string& value);
        void Write(const std::vector<std::string>& vector);
        void Write(const std::vector<uint32_t>& vector);
        void Write(const std::vector<XMFLOAT3>& vector);
        void Write(const std::vector<XMFLOAT2>& vector);
        void Write(const std::vector<unsigned char>& vector);
        void Write(const std::vector<std::byte>& vector);
        void Skip(uint32_t skipAmount);

        // Reading
        template <class T, class = typename std::enable_if <
            std::is_same<T, bool>::value ||
            std::is_same<T, unsigned char>::value ||
            std::is_same<T, int>::value ||
            std::is_same<T, long>::value ||
            std::is_same<T, long long>::value ||
            std::is_same<T, uint8_t>::value ||
            std::is_same<T, uint16_t>::value ||
            std::is_same<T, uint32_t>::value ||
            std::is_same<T, uint64_t>::value ||
            std::is_same<T, unsigned long>::value ||
            std::is_same<T, unsigned long long>::value ||
            std::is_same<T, float>::value ||
            std::is_same<T, double>::value ||
            std::is_same<T, long double>::value ||
            std::is_same<T, std::byte>::value ||
            std::is_same<T, XMFLOAT2>::value ||
            std::is_same<T, XMFLOAT3>::value>::type>
        void Read(T* value)
        {
            m_InputStream.read(reinterpret_cast<char*>(value), sizeof(T));
        }

        void Read(std::string* values);
        void Read(std::vector<std::string>* vector);
        void Read(std::vector<XMFLOAT3>* vector);
        void Read(std::vector<XMFLOAT2>* vector);
        void Read(std::vector<uint32_t>* vector);
        void Read(std::vector<unsigned char>* vector);
        void Read(std::vector<std::byte>* vector);

        // Reading with explicit type definition
        template <class T, class = typename std::enable_if <
            std::is_same<T, bool>::value ||
            std::is_same<T, unsigned char>::value ||
            std::is_same<T, int>::value ||
            std::is_same<T, long>::value ||
            std::is_same<T, long long>::value ||
            std::is_same<T, uint8_t>::value ||
            std::is_same<T, uint16_t>::value ||
            std::is_same<T, uint32_t>::value ||
            std::is_same<T, uint64_t>::value ||
            std::is_same<T, unsigned long>::value ||
            std::is_same<T, unsigned long long>::value ||
            std::is_same<T, float>::value ||
            std::is_same<T, double>::value ||
            std::is_same<T, long double>::value ||
            std::is_same<T, std::byte>::value ||
            std::is_same<T, std::string>::value>::type>
        T ReadAs()
        {
            T value;
            Read(&value);
            return value;
        }

    private:
        std::ofstream m_OutputStream;
        std::ifstream m_InputStream;
        uint32_t m_SerializerFlags;
        bool m_IsStreamOpen;
    };
}