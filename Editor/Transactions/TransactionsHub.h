#pragma once
#include <unordered_map>
#include <functional>
#include <deque>
#include "TransactionPayload.h"

#define SUBSCRIBE_TO_TRANSACTION_EVENT(eventType, eventFunction)   TransactionsHub::GetInstance().Subscribe(eventType, eventFunction);
#define TRANSACTION_HUB_FIRE_EVENT(eventType, eventPayload)        TransactionsHub::GetInstance().Fire(eventType, eventPayload);

using Transaction = std::function<void(const TransactionPayload& payload)>;

class TransactionsHub
{
public:
    static TransactionsHub& GetInstance()
    {
        static TransactionsHub hubInstance;
        return hubInstance;
    }

    void Subscribe(const TransactionType transactionType, Transaction&& subscriberFunction)
    {
        m_Subscribers[transactionType].push_back(std::move(subscriberFunction));
    }

    void Fire(const TransactionType transactionType, const TransactionPayload& payload = { 0, TransactionType::TransactionType_Unknown })
    {
        if (m_Subscribers.find(transactionType) == m_Subscribers.end())
        {
            return;
        }

        for (const Transaction& transaction : m_Subscribers[transactionType])
        {
            transaction(payload);
        }
    }

    void PushTransaction(TransactionPayload&& transactionPayload)
    {
        m_Transactions.push_back(std::move(transactionPayload));
        m_TransactionsIndex = m_Transactions.size() - 1; // To be used for indexing.
    }

    void Redo()
    {
        if (m_TransactionsIndex + 1 < m_Transactions.size() && m_Transactions.size() > 0) // If there is anything to redo at all...
        {
            TransactionPayload& payload = m_Transactions[++m_TransactionsIndex];

            switch (payload.GetTransactionType())
            {
                case TransactionType::TransactionType_Add:
                    ///
                    break;

                case TransactionType::TransactionType_Delete:
                    ///
                    break;

                case TransactionType::TransactionType_ValueChange:

                    Transaction_ValueChange valueChangeInformation = std::get<Transaction_ValueChange>(payload.GetData());
                    valueChangeInformation.m_ValueFunction(valueChangeInformation.m_NewValue);
                    break;
            }
        }
    }

    void Undo()
    {
        if (m_TransactionsIndex >= 0) // If there is anything to undo at all...
        {
            TransactionPayload& payload = m_Transactions[m_TransactionsIndex];

            switch (payload.GetTransactionType())
            {
            case TransactionType::TransactionType_Add:
                ///
                break;

            case TransactionType::TransactionType_Delete:
                ///
                break;

            case TransactionType::TransactionType_ValueChange:

                Transaction_ValueChange valueChangeInformation = std::get<Transaction_ValueChange>(payload.GetData());
                valueChangeInformation.m_ValueFunction(valueChangeInformation.m_OldValue);
                break;
            }

            m_TransactionsIndex--;
        }
    }

    void Clear()
    {
        m_Subscribers.clear();
    }

private:
    std::unordered_map<TransactionType, std::vector<Transaction>> m_Subscribers;
    std::vector<TransactionPayload> m_Transactions;
    int m_TransactionsIndex = -1;
};