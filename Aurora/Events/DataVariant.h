#pragma once
#include <variant>
#include <DirectXMath.h>

namespace Aurora
{
#define _VariantTypes     \
    char,                 \
    unsigned char,        \
    int,                  \
    uint32_t,             \
    bool,                 \
    float,                \
    double,               \
    void*,                \
    DirectX::XMFLOAT4,    \
    DirectX::XMFLOAT3,    \
    DirectX::XMFLOAT2

//  #define VariantTypes std::variant<_VariantTypes>

    class DataVariant
    {
    public:
        DataVariant() = default;
        ~DataVariant() = default;

        // Copy Constructor 1
        DataVariant(const DataVariant& otherVariant) { m_Variant = otherVariant.GetVariantRaw(); }

        // Copy Constructor 2
        template<typename T, typename = std::enable_if<!std::is_same<T, DataVariant>::value>>
        DataVariant(T value) { m_Variant = value; }

        // Assignment Operator 1
        DataVariant& operator=(const DataVariant& otherVariant) { *this = otherVariant; return *this; }

        // Assignment Operator 2
        template<typename T, typename = std::enable_if<!std::is_same<T, DataVariant>::value>>
        DataVariant& operator=(T otherVariant) { return m_Variant = otherVariant; }

        const std::variant<_VariantTypes>& GetVariantRaw() const { return m_Variant; }

        template<typename T>
        const T& Get() const { std::get<T>(m_Variant); }

    private:
        std::variant<_VariantTypes> m_Variant;
    };
}