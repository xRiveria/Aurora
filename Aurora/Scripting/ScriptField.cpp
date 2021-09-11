#include "Aurora.h"
#include "ScriptField.h"
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/attrdefs.h>
#include "ScriptInstance.h"

namespace Aurora
{
    PublicField::PublicField(const std::string& fieldName, const std::string& fieldTypeName, FieldType fieldType, bool isReadOnly) : m_FieldName(fieldName), m_FieldTypeName(fieldTypeName), m_FieldType(fieldType), m_IsReadOnly(isReadOnly)
    {
        if (m_FieldType != FieldType::String)
        {
            m_StoredValueBuffer = AllocateBuffer(m_FieldType);
        }
    }

    PublicField::PublicField(const PublicField& otherField) : m_FieldName(otherField.m_FieldName), m_FieldTypeName(otherField.m_FieldTypeName), m_FieldType(otherField.m_FieldType), m_IsReadOnly(otherField.m_IsReadOnly)
    {
        if (m_FieldType != FieldType::String)
        {
            m_StoredValueBuffer = AllocateBuffer(m_FieldType);
            memcpy(m_StoredValueBuffer, otherField.m_StoredValueBuffer, GetFieldTypeSize(m_FieldType));
        }
        else
        {
            m_StoredValueBuffer = otherField.m_StoredValueBuffer;
        }

        m_MonoClassField = otherField.m_MonoClassField;
        m_MonoProperty = otherField.m_MonoProperty;
    }

    PublicField& PublicField::operator=(const PublicField& otherField)
    {
        if (&otherField != this)
        {
            m_FieldName = otherField.m_FieldName;
            m_FieldTypeName = otherField.m_FieldTypeName;
            m_FieldType = otherField.m_FieldType;
            m_IsReadOnly = otherField.m_IsReadOnly;
            m_MonoClassField = otherField.m_MonoClassField;
            m_MonoProperty = otherField.m_MonoProperty;

            if (m_FieldType != FieldType::String)
            {
                m_StoredValueBuffer = AllocateBuffer(m_FieldType);
                memcpy(m_StoredValueBuffer, otherField.m_StoredValueBuffer, GetFieldTypeSize(m_FieldType));
            }
            else
            {
                m_StoredValueBuffer = otherField.m_StoredValueBuffer;
            }
        }

        return *this;
    }

    PublicField::PublicField(PublicField&& otherField)
    {
        m_FieldName = std::move(otherField.m_FieldName);
        m_FieldTypeName = std::move(otherField.m_FieldTypeName);
        m_FieldType = otherField.m_FieldType;
        m_IsReadOnly = otherField.m_IsReadOnly;
        m_MonoClassField = otherField.m_MonoClassField;
        m_MonoProperty = otherField.m_MonoProperty;
        m_StoredValueBuffer = otherField.m_StoredValueBuffer;

        otherField.m_MonoProperty = nullptr;
        otherField.m_MonoClassField = nullptr;

        if (m_FieldType != FieldType::String)
        {
            otherField.m_StoredValueBuffer = nullptr;
        }
    }

    PublicField::~PublicField()
    {
        if (m_FieldType != FieldType::String)
        {
            delete[] m_StoredValueBuffer;
        }
    }

    void PublicField::SetStoredValueRaw(void* sourceValue)
    {
        if (m_IsReadOnly)
        {
            AURORA_WARNING(LogLayer::Scripting, "Field value is Read Only.");
            return;
        }

        uint32_t typeSize = GetFieldTypeSize(m_FieldType);
        memcpy(m_StoredValueBuffer, sourceValue, typeSize);
    }

    void PublicField::SetRuntimeValueRaw(ScriptInstanceData& scriptInstance, void* sourceValue)
    {
        AURORA_ASSERT(scriptInstance.GetInstance());

        if (m_IsReadOnly)
        {
            AURORA_WARNING(LogLayer::Scripting, "Field value is Read Only.");
            return;
        }

        if (m_MonoProperty)
        {
            void* parameters[] = { sourceValue };
            mono_property_set_value(m_MonoProperty, scriptInstance.GetInstance(), parameters, nullptr);
        }
        else
        {
            mono_field_set_value(scriptInstance.GetInstance(), m_MonoClassField, sourceValue);
        }
    }

    void* PublicField::GetRuntimeValueRaw(ScriptInstanceData& scriptInstance)
    {
        AURORA_ASSERT(scriptInstance.GetInstance());

        uint8_t* outValue = nullptr;
        mono_field_get_value(scriptInstance.GetInstance(), m_MonoClassField, outValue);
        return outValue;
    }

    uint8_t* PublicField::AllocateBuffer(FieldType fieldType)
    {
        uint32_t size = GetFieldTypeSize(fieldType);
        uint8_t* buffer = new uint8_t[size];
        memset(buffer, 0, size);
        return buffer;
    }

    void PublicField::SetStoredValue_Internal(void* value)
    {
        if (m_IsReadOnly)
        {
            AURORA_WARNING(LogLayer::Scripting, "Field value is Read Only.");
            return;
        }

        uint32_t fieldTypeSize = GetFieldTypeSize(m_FieldType);
        memcpy(m_StoredValueBuffer, value, fieldTypeSize);
    }

    void PublicField::GetStoredValue_Internal(void* outValue) const
    {
        uint32_t fieldTypeSize = GetFieldTypeSize(m_FieldType);
        memcpy(outValue, m_StoredValueBuffer, fieldTypeSize);
    }

    void PublicField::SetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, void* sourceValue)
    {
        AURORA_ASSERT(scriptInstance.GetInstance());

        if (m_IsReadOnly)
        {
            AURORA_WARNING(LogLayer::Scripting, "Field value is Read Only.");
            return;
        }

        if (m_MonoProperty)
        {
            void* parameters[] = { sourceValue };
            mono_property_set_value(m_MonoProperty, scriptInstance.GetInstance(), parameters, nullptr);
        }
        else
        {
            mono_field_set_value(scriptInstance.GetInstance(), m_MonoClassField, sourceValue);
        }
    }

    void PublicField::SetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, const std::string& sourceValue)
    {
        AURORA_ASSERT(scriptInstance.GetInstance());

        if (m_IsReadOnly)
        {
            AURORA_WARNING(LogLayer::Scripting, "Field value is Read Only.");
            return;
        }

        MonoString* monoString = mono_string_new(mono_domain_get(), sourceValue.c_str());

        if (m_MonoProperty)
        {
            void* parameters[] = { monoString };
            mono_property_set_value(m_MonoProperty, scriptInstance.GetInstance(), parameters, nullptr);
        }
        else
        {
            mono_field_set_value(scriptInstance.GetInstance(), m_MonoClassField, monoString);
        }
    }

    void PublicField::GetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, void* outValue) const
    {
        AURORA_ASSERT(scriptInstance.GetInstance());

        if (m_MonoProperty)
        {
            MonoObject* result = mono_property_get_value(m_MonoProperty, scriptInstance.GetInstance(), nullptr, nullptr);
            memcpy(outValue, mono_object_unbox(result), GetFieldTypeSize(m_FieldType));
        }
        else
        {
            mono_field_get_value(scriptInstance.GetInstance(), m_MonoClassField, outValue);
        }
    }

    void PublicField::GetRuntimeValue_Internal(ScriptInstanceData& scriptInstance, std::string& outValue) const
    {
        AURORA_ASSERT(scriptInstance.GetInstance());

        MonoString* monoString;
        if (m_MonoProperty)
        {
            monoString = (MonoString*)mono_property_get_value(m_MonoProperty, scriptInstance.GetInstance(), nullptr, nullptr);
        }
        else
        {
            mono_field_get_value(scriptInstance.GetInstance(), m_MonoClassField, &monoString);
        }

        outValue = mono_string_to_utf8(monoString);
    }
}