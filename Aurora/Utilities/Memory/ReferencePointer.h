#pragma once
#include "PointerUtilities.h"
#include <iostream>

namespace Aurora
{
    template <typename T>
    class ReferencePointer
    {
    public:
        ReferencePointer() : m_InternalInstance(nullptr)
        {

        }

        ReferencePointer(std::nullptr_t nullPointer) : m_InternalInstance(nullptr)
        {

        }

        ReferencePointer(T* instance)
        {
            m_InternalInstance = instance;
            m_Counter = new PointerCounter();
            (*m_Counter)++;
        }

        ReferencePointer(const ReferencePointer& otherPointer)
        {
            m_InternalInstance = otherPointer.m_InternalInstance;
            m_Counter = otherPointer.m_Counter;

            if (m_InternalInstance != nullptr)
            {
                (*m_Counter)++; // Increment count of counter exists.
            }
        }

        ReferencePointer& operator=(const ReferencePointer& otherPointer)
        {
            CleanUp(); // In case the existing memory here already points elsewhere.

            m_InternalInstance = otherPointer.m_InternalInstance;
            m_Counter = otherPointer.m_Counter;

            if (m_InternalInstance != nullptr)
            {
                (*m_Counter)++; // Increment count of counter exists.
            }

            return *this;
        }

        void Reset()
        {
            CleanUp();
        }

        unsigned int GetUseCount()
        {
            return m_Counter->GetCount();
        }

        bool IsInitialized()
        {
            if (m_InternalInstance != nullptr)
            {
                return true;
            }

            return false;
        }

        T* Get() const
        {
            return m_InternalInstance;
        }

        T* operator->() const
        {
            return m_InternalInstance;
        }

        T& operator*() const
        {
            return *m_InternalInstance;
        }

        ~ReferencePointer()
        {

            CleanUp();
        }

        friend std::ostream& operator<<(std::ostream& stream, ReferencePointer<T>& pointer)
        {
            stream << "Address Pointed: " << pointer.Get() << "\n";
            stream << "Use Count: " << pointer.GetUseCount() << "\n";

            return stream;
        }

    private:
        void CleanUp()
        {
            if (m_InternalInstance) // Only cleanup if our instance exists.
            {
                (*m_Counter)--;

                if (m_Counter->GetCount() == 0)
                {
                    delete m_Counter;
                    m_Counter = nullptr;

                    delete m_InternalInstance;
                    m_InternalInstance = nullptr;
                }
            }
        }

    private:
        T* m_InternalInstance = nullptr;
        PointerCounter* m_Counter = nullptr; // Pointer as it allows us to share it across multiple shared pointers and allow for access/modification.
    };

    template<typename T>
    ReferencePointer<T> MakeReference(T value)
    {
        ReferencePointer<T> newReference(new T(value));
        return newReference;
    }
}