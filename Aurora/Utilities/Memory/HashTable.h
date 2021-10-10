#pragma once
#include <iostream>

namespace Aurora
{
    template<typename K, typename V>
    class HashNode
    {
    public:
        HashNode(const K& key, const V& value)
        {
            m_Key = key;
            m_Value = value;
            m_NextNode = nullptr;
        }

        K GetKey() const
        {
            return m_Key;
        }

        V GetValue() const
        {
            return m_Value;
        }

        HashNode* GetNextNode() const
        {
            return m_NextNode;
        }

        void SetKey(K key)
        {
            m_Key = key;
        }

        void SetValue(V value)
        {
            m_Value = value;
        }

        void SetNextNode(HashNode* nextNode)
        {
            m_NextNode = nextNode;
        }

    private:
        K m_Key;
        V m_Value;
        HashNode* m_NextNode; // Next bucket with the same key.
    };

    // Default hash function class.
    template<typename K>
    struct KeyHashFunction
    {
        size_t operator()(const K& key, const int& hashSize = 1000) const
        {
            return std::hash<K>() (key) % hashSize;
        }
    };

    template<typename K, typename V, typename F = KeyHashFunction<K>>
    class HashMap
    {
    public:
        HashMap()
        {
            m_Capacity = 10; // Initial capacity of 10.
            m_Size = 0;
            m_HashTable = new HashNode<K, V>*[m_Capacity];

            // Iniitalize all elements of our hash table to null.
            for (int i = 0; i < m_Capacity; i++)
            {
                m_HashTable[i] = nullptr;
            }
        }

        HashMap(size_t size = 10)
        {
            m_Capacity = size; // Initial capacity of 10.
            m_Size = 0;
            m_HashTable = new HashNode<K, V>*[m_Capacity];

            // Iniitalize all elements of our hash table to null.
            for (int i = 0; i < m_Capacity; i++)
            {
                m_HashTable[i] = nullptr;
            }
        }

        ~HashMap()
        {
            for (size_t i = 0; i < m_Capacity; i++)
            {
                delete m_HashTable[i];
            }

            delete[] m_HashTable;
        }
        
        bool Insert(const K& key, const V& value)
        {
            if (m_Size + 1 > m_Capacity) // Ensure that we have enough capacity for this new addition.
            {
                return false;
            }

            size_t hashValue = m_HashFunction(key, m_Capacity);
            size_t startTablePosition = hashValue;
            size_t endTablePosition = (startTablePosition > 0) ? ((startTablePosition - 1) % m_Capacity) : m_Capacity - 1;

            while (startTablePosition != endTablePosition)
            {
                // If we found an empty bucket...
                if (m_HashTable[startTablePosition] == nullptr)
                {
                    break;
                }
                else if (m_HashTable[startTablePosition] != nullptr && m_HashTable[startTablePosition]->GetKey() == key)
                {
                    return false; // Return from the method.
                }

                startTablePosition = (startTablePosition + 1) % m_Capacity;
            }

            // There's no empty buckets avaliable. Exit.
            if (startTablePosition == endTablePosition)
            {
                return false;
            }

            m_HashTable[startTablePosition] = new HashNode<K, V>(key, value);
            m_Size += 1;
            
            return true;
        }

        V GetValue(const K& key)
        {
            if (m_Size == 0)
            {
                std::cout << "Key not found.\n";
            }

            size_t hashValue = m_HashFunction(key, m_Capacity);
            size_t startTablePosition = hashValue;
            size_t endTablePosition = (startTablePosition > 0) ? ((startTablePosition - 1) % m_Capacity) : m_Capacity - 1;
            
            while (startTablePosition != endTablePosition)
            {
                if (m_HashTable[startTablePosition] == nullptr)
                {
                    std::cout << "Key not found.\n";
                }
                else if (m_HashTable[startTablePosition] != nullptr && m_HashTable[startTablePosition]->GetKey() == key)
                {
                    return m_HashTable[startTablePosition]->GetValue();
                }

                startTablePosition = (startTablePosition + 1) % m_Capacity;
            }

            std::cout << "Key not found.\n";
        }

        void PrintTable()
        {
            for (size_t i = 0; i < m_Capacity; i++)
            {
                if (m_HashTable[i] != nullptr)
                {
                    std::cout << i << ": " << m_HashTable[i]->GetKey() << " : " << m_HashTable[i]->GetValue() << "\n";
                }
            }
        }

        size_t GetCapacity() const
        {
            return m_Capacity;
        }

        size_t GetSize() const
        {
            return m_Size;
        }

    private:
        HashNode<K, V>** m_HashTable; // Our table array.
        size_t m_Capacity;
        size_t m_Size; // Current size.
        F m_HashFunction;
    };
}