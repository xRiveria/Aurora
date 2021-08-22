#include "Aurora.h"
#include "BinarySerializer.h"

namespace Aurora
{

    BinarySerializer::BinarySerializer(const std::string& filePath, uint32_t serializerFlags)
    {
        m_IsStreamOpen = false;
        m_SerializerFlags = serializerFlags;

        int streamFlags = std::ios::binary; // Accesses the file as a binary file - no formatting is done its contents (conversion, insertion, whatsoever). The loss of information occurs and data is saved as it is. Its also indecipherable when viewed with an editor.
        streamFlags |= (serializerFlags & SerializerFlag::SerializerMode_Read) ? std::ios::in : 0;     // Opens the file for reading.
        streamFlags |= (serializerFlags & SerializerFlag::SerializerMode_Write) ? std::ios::out : 0;   // Opens the file for writing.
        streamFlags |= (serializerFlags & SerializerFlag::SerializerMode_Append) ? std::ios::app : 0;  // Opens the file for appending.

        if (m_SerializerFlags & SerializerFlag::SerializerMode_Write)
        {
            m_OutputStream.open(filePath, streamFlags);
            if (m_OutputStream.fail())
            {
                AURORA_ERROR(LogLayer::Serialization, "Failed to open \"%s\" for writing.", filePath.c_str());
                return;
            }
        }
        else if (m_SerializerFlags & SerializerFlag::SerializerMode_Read)
        {
            m_InputStream.open(filePath, streamFlags);
            if (m_InputStream.fail())
            {
                AURORA_ERROR(LogLayer::Serialization, "Failed to open \"%s\" for reading.", filePath.c_str());
                return;
            }
        }

        m_IsStreamOpen = true;
    }

    BinarySerializer::~BinarySerializer()
    {
        CloseStream();
    }

    void BinarySerializer::CloseStream()
    {
        if (m_SerializerFlags & SerializerFlag::SerializerMode_Write)
        {
            m_OutputStream.flush();
            m_OutputStream.close();
        }
        else if (m_SerializerFlags & SerializerFlag::SerializerMode_Read)
        {
            m_InputStream.clear();
            m_InputStream.close();
        }
    }

    void BinarySerializer::Write(const std::string& value)
    {
        const uint32_t length = static_cast<uint32_t>(value.length());
        Write(length);

        m_OutputStream.write(const_cast<char*>(value.c_str()), length);
    }

    void BinarySerializer::Write(const std::vector<std::string>& vector)
    {
        const uint32_t size = static_cast<uint32_t>(vector.size());
        Write(size);

        for (uint32_t i = 0; i < size; i++)
        {
            Write(vector[i]);
        }
    }

    void BinarySerializer::Write(const std::vector<XMFLOAT3>& vector)
    {
        const uint32_t size = static_cast<uint32_t>(vector.size());
        Write(size);
        m_OutputStream.write(reinterpret_cast<const char*>(&vector[0]), sizeof(XMFLOAT3) * size);
    }

    void BinarySerializer::Write(const std::vector<XMFLOAT2>& vector)
    {
        const uint32_t size = static_cast<uint32_t>(vector.size());
        Write(size);
        m_OutputStream.write(reinterpret_cast<const char*>(&vector[0]), sizeof(XMFLOAT2) * size);
    }

    void BinarySerializer::Write(const std::vector<uint32_t>& vector)
    {
        const uint32_t size = static_cast<uint32_t>(vector.size());
        Write(size);
        m_OutputStream.write(reinterpret_cast<const char*>(&vector[0]), sizeof(uint32_t) * size);
    }

    void BinarySerializer::Write(const std::vector<unsigned char>& vector)
    {
        const uint32_t size = static_cast<uint32_t>(vector.size());
        Write(size);
        m_OutputStream.write(reinterpret_cast<const char*>(&vector[0]), sizeof(unsigned char) * size);
    }

    void BinarySerializer::Write(const std::vector<std::byte>& vector)
    {
        const uint32_t size = static_cast<uint32_t>(vector.size());
        Write(size);
        m_OutputStream.write(reinterpret_cast<const char*>(&vector[0]), sizeof(std::byte) * size);
    }

    void BinarySerializer::Skip(uint32_t skipAmount)
    {
        // Set the seek cursor to offset skipAmount from the current position.
        if (m_SerializerFlags & SerializerFlag::SerializerMode_Write)
        {
            m_OutputStream.seekp(skipAmount, std::ios::cur); // skipAmount of chars from the current position.
        }
        else if (m_SerializerFlags & SerializerFlag::SerializerMode_Read)
        {
            m_InputStream.ignore(skipAmount, std::ios::cur);
        }
    }

    void BinarySerializer::Read(std::string* values)
    {
        uint32_t length = 0;
        Read(&length);

        values->resize(length);
        m_InputStream.read(const_cast<char*>(values->c_str()), length);
    }

    void BinarySerializer::Read(std::vector<std::string>* vector)
    {
        if (!vector)
        {
            return;
        }

        vector->clear();
        vector->shrink_to_fit();

        uint32_t size = 0;
        Read(&size);

        std::string readString;
        for (uint32_t i = 0; i < size; i++)
        {
            Read(&readString);
            vector->emplace_back(readString);
        }
    }

    void BinarySerializer::Read(std::vector<XMFLOAT2>* vector)
    {
        if (!vector)
        {
            return;
        }

        vector->clear();
        vector->shrink_to_fit();

        const uint32_t size = ReadAs<uint32_t>();

        vector->reserve(size);
        vector->resize(size);

        m_InputStream.read(reinterpret_cast<char*>(vector->data()), sizeof(XMFLOAT2) * size);
    }

    void BinarySerializer::Read(std::vector<XMFLOAT3>* vector)
    {
        if (!vector)
        {
            return;
        }

        vector->clear();
        vector->shrink_to_fit();

        const uint32_t size = ReadAs<uint32_t>();

        vector->reserve(size);
        vector->resize(size);

        m_InputStream.read(reinterpret_cast<char*>(vector->data()), sizeof(XMFLOAT3) * size);
    }

    void BinarySerializer::Read(std::vector<uint32_t>* vector)
    {
        if (!vector)
        {
            return;
        }

        vector->clear();
        vector->shrink_to_fit();

        const uint32_t size = ReadAs<uint32_t>();

        vector->reserve(size);
        vector->resize(size);

        m_InputStream.read(reinterpret_cast<char*>(vector->data()), sizeof(uint32_t) * size);
    }

    void BinarySerializer::Read(std::vector<unsigned char>* vector)
    {
        if (!vector)
        {
            return;
        }

        vector->clear();
        vector->shrink_to_fit();

        const uint32_t size = ReadAs<uint32_t>();

        vector->reserve(size);
        vector->resize(size);

        m_InputStream.read(reinterpret_cast<char*>(vector->data()), sizeof(unsigned char) * size);
    }

    void BinarySerializer::Read(std::vector<std::byte>* vector)
    {
        if (!vector)
        {
            return;
        }

        vector->clear();
        vector->shrink_to_fit();

        const uint32_t size = ReadAs<uint32_t>();

        vector->reserve(size);
        vector->resize(size);

        m_InputStream.read(reinterpret_cast<char*>(vector->data()), sizeof(std::byte) * size);
    }
}