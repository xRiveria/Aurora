#pragma once
#include <variant>
#include <string>

enum class TransactionType
{
    TransactionType_Add,
    TransactionType_Delete,
    TransactionType_ValueChange,
    TransactionType_Unknown
};

struct Transaction_ValueChange
{
    std::variant<bool, uint32_t, float> m_NewValue;
    std::variant<bool, uint32_t, float> m_OldValue;
    std::function<void(std::variant<bool, uint32_t, float> value)> m_ValueFunction;
};

struct Transaction_AddObject
{
    uint32_t m_ObjectUUID;
};

struct Transaction_DeleteObject
{
    uint32_t m_ObjectUUID;
};

struct TransactionPayload
{
    TransactionPayload() = default;
    ~TransactionPayload() = default;
    
    // We will only instantiate the template if its type isn't of TransactionPayload.
    template<typename T, typename = std::enable_if<!std::is_same<T, TransactionPayload>::value>>
    TransactionPayload(const T value, TransactionType transactionType) 
    { 
        m_Data = value; 
        m_TransactionType = transactionType;
    }

    const std::variant<Transaction_AddObject, Transaction_DeleteObject, Transaction_ValueChange>& GetData() const { return m_Data; }
    TransactionType GetTransactionType() const { return m_TransactionType; }

private:
    std::variant<Transaction_AddObject, Transaction_DeleteObject, Transaction_ValueChange> m_Data;
    TransactionType m_TransactionType = TransactionType::TransactionType_Unknown;
};