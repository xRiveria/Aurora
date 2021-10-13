#pragma once
#include "../Scene/Components/IComponent.h"
#include "TransactionsHub.h"
#include "TransactionPayload.h"

class DifferentiatorTool
{
public:
    DifferentiatorTool() = default;

    // Pass in any component type.
    void TakeSnapshot(Aurora::IComponent& component)
    {
        m_Component = &component;
        m_TransactionAttributes = component.GetTransactionAttributes();
    }

    // Produces changes as transactions to the Transactions Hub via an attribute shallow copy.
    void CommitChanges(const std::string& commitDescription)
    {
        std::vector<std::variant<bool, uint32_t, float>> transactionAttributes = m_Component->GetTransactionAttributes();
        for (int i = 0; i < transactionAttributes.size(); i++)
        {
            if (transactionAttributes[i] != m_TransactionAttributes[i])
            {
                Transaction_ValueChange valueChangePayload;

                valueChangePayload.m_OldValue = m_TransactionAttributes[i];
                valueChangePayload.m_NewValue = transactionAttributes[i];
                valueChangePayload.m_ValueFunction = m_Component->GetTransactionFunctions()[i];
                TransactionsHub::GetInstance().PushTransaction(TransactionPayload(valueChangePayload, TransactionType::TransactionType_ValueChange));
            }
        }
    }

private: 
    // Shallow attribute copy.
    std::vector<std::variant<bool, uint32_t, float>> m_TransactionAttributes;
    Aurora::IComponent* m_Component;
};