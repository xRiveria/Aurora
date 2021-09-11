#pragma once
#include <string>
#include <unordered_map>

extern "C"
{
    typedef struct _MonoClassField MonoClassField;
    typedef struct _MonoProperty MonoProperty;
}

namespace Aurora
{
    enum class FieldType
    {
        None = 0,
        Float,
        Integer,
        UnsignedInteger,
        String,
        Vector2,
        Vector3,
        Vector4
    };

    inline const char* FieldTypeToString(FieldType fieldType)
    {
        switch (fieldType)
        {
            case FieldType::Float:           return "Float";
            case FieldType::Integer:         return "Integer";
            case FieldType::UnsignedInteger: return "UnsignedInteger";
            case FieldType::String:          return "String";
            case FieldType::Vector2:         return "Vector2";
            case FieldType::Vector3:         return "Vector3";
            case FieldType::Vector4:         return "Vector4";
        }

        AURORA_ASSERT(false, "Unknown Field Type!");
        return "Unknown Field Type";
    }

    static uint32_t GetFieldTypeSize(FieldType fieldType)
    {
        switch (fieldType)
        {
            case FieldType::Float:           return 4;
            case FieldType::Integer:         return 4;
            case FieldType::UnsignedInteger: return 4;
            case FieldType::String:          return 8;
            case FieldType::Vector2:         return 4 * 2;
            case FieldType::Vector3:         return 4 * 3;
            case FieldType::Vector4:         return 4 * 4;
        }

        AURORA_ASSERT(false, "Unknown Field Type!");
        return 0;
    }

    struct ScriptInstanceData;

    struct PublicField
    {
        std::string m_FieldName;
        std::string m_FieldTypeName;
        FieldType m_FieldType;
        bool m_IsReadOnly;

        PublicField() = default;
        PublicField(const std::string& fieldName, const std::string& fieldTypeName, FieldType fieldType, bool isReadOnly = false);

        PublicField(const PublicField& otherField);

        PublicField& operator=(const PublicField& otherField);

        PublicField& operator=(PublicField&& otherField) = default;

        PublicField(PublicField&& otherField);

        ~PublicField();

        void CopyStoredValueToRuntime(ScriptInstanceData& scriptInstance);
        void CopyStoredValueFromRuntime(ScriptInstanceData& scriptInstance);

        template<typename T>
        T GetStoredValue() const
        {
            T value;
            GetStoredValue_Internal(&value);
            return value;
        }

        template<>
        const std::string& GetStoredValue() const
        {
            return *(std::string*)m_StoredValueBuffer;
        }

        template<typename T>
        void SetStoredValue(T value)
        {
            SetStoredValue_Internal(&value);
        }

        template<>
        void SetStoredValue(const std::string& value)
        {
            (*(std::string*)m_StoredValueBuffer).assign(value);
        }

        template<typename T>
        T GetRuntimeValue(ScriptInstanceData& scriptInstance) const
        {
            T value;
            GetRuntimeValue_Internal(scriptInstance, &value);
            return value;
        }

        template<>
        std::string GetRuntimeValue(ScriptInstanceData& scriptInstance) const
        {
            std::string value;
            GetRuntimeValue_Internal(scriptInstance, value);
            return value;
        }

        template<typename T>
        void SetRuntimeValue(ScriptInstanceData& scriptInstance, T value)
        {
            SetRuntimeValue_Internal(scriptInstance, &value);
        }

        template<>
        void SetRuntimeValue(ScriptInstanceData& scriptInstance, const std::string& value)
        {
            SetRuntimeValue_Internal(scriptInstance, value);
        }

        void SetStoredValueRaw(void* sourceValue);

        void* GetStoredValueRaw() { return m_StoredValueBuffer; }

        void SetRuntimeValueRaw(ScriptInstanceData& scriptInstance, void* sourceValue);
        void* GetRuntimeValueRaw(ScriptInstanceData& scriptInstance);
        
    private:
        // Our field will either have a mono field or mono property.
        MonoClassField* m_MonoClassField = nullptr;
        MonoProperty* m_MonoProperty = nullptr;
        uint8_t* m_StoredValueBuffer = nullptr;

    private:
        uint8_t* AllocateBuffer(FieldType fieldType);
        void SetStoredValue_Internal(void* value);
        void GetStoredValue_Internal(void* outValue) const;
        void SetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, void* sourceValue);
        void SetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, const std::string& sourceValue);
        void GetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, void* outValue) const;
        void GetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, std::string& outValue) const;

    private:
        friend class Scripting;
    };

    using ScriptFieldMapping = std::unordered_map<std::string, std::unordered_map<std::string, PublicField>>;
}