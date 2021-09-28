#pragma once

namespace Aurora
{
    class PointerCounter
    {
    public:
        PointerCounter() : m_CounterInternal(0) { };
        PointerCounter(const PointerCounter& otherCounter) = delete;
        PointerCounter(PointerCounter&& otherCounter) = delete;
        PointerCounter& operator=(const PointerCounter& otherCounter) = delete;

        ~PointerCounter() {}

        void Reset()
        {
            m_CounterInternal = 0;
        }

        unsigned int GetCount() const
        {
            return m_CounterInternal;
        }

        void operator++()
        {
            m_CounterInternal++;
        }

        void operator++(int)
        {
            m_CounterInternal++;
        }

        void operator--()
        {
            m_CounterInternal--;
        }

        void operator--(int)
        {
            m_CounterInternal--;
        }

    private:
        unsigned int m_CounterInternal {};
    };
}