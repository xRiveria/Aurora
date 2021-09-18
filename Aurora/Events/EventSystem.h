#pragma once
#include <unordered_map>
#include <functional>
#include "DataVariant.h"

namespace Aurora
{
#define AURORA_EVENT_INSTANTIATE(function)                      [this](const Aurora::DataVariant& variant) { function(); }
#define AURORA_EVENT_INSTANTIATE_STATIC(function)               [](const Aurora::DataVariant& variant) { function(); }

#define AURORA_EVENT_INSTANTIATE_VARIANT(function)              [this](const Aurora::DataVariant& variant) { function(variant); }
#define AURORA_EVENT_INSTANTIATE_VARIANT_STATIC(function)       [](const Aurora::DataVariant& variant) { function(variant); }

#define AURORA_FIRE_EVENT(eventType)                            Aurora::EventSystem::GetInstance().Fire(eventType)
#define AURORA_FIRE_EVENT_DATA(eventType, eventData)            Aurora::EventSystem::GetInstance().Fire(eventType, eventData)

#define AURORA_SUBSCRIBE_TO_EVENT(eventType, eventFunction)     Aurora::EventSystem::GetInstance().Subscribe(eventType, eventFunction);
#define AURORA_UNSUBSCRIBE_FROM_EVENT(eventType, eventFunction) Aurora::EventSystem::GetInstance().Unsubscribe(eventType, eventFunction);

    using Subscriber = std::function<void(const DataVariant&)>;

    enum class EventType
    {
        SceneSaveStart,
        SceneSaveEnd,
        SceneLoadStart,
        SceneLoadEnd,
        ScenePreClear,
        SceneClear,
    };

    class EventSystem
    {
    public:
        static EventSystem& GetInstance()
        {
            static EventSystem eventSystemInstance;
            return eventSystemInstance;
        }

        void Subscribe(const EventType eventType, Subscriber&& subscriberFunction)
        {
            m_Subscribers[eventType].push_back(std::forward<Subscriber>(subscriberFunction));
        }

        void Unsubscribe(const EventType eventType, Subscriber&& subscriberFunction)
        {
            const size_t functionAddress = *reinterpret_cast<long*>(reinterpret_cast<char*>(&subscriberFunction));
            std::vector<Subscriber>& subscribers = m_Subscribers[eventType];

            for (auto it = subscribers.begin(); it != subscribers.end();)
            {
                const size_t subscriberFunctionAddress = *reinterpret_cast<long*>(reinterpret_cast<char*>(&(*it)));
                if (subscriberFunctionAddress == functionAddress)
                {
                    it = subscribers.erase(it);
                    return;
                }
            }
        }

        void Fire(const EventType eventType, const DataVariant& data = 0)
        {
            if (m_Subscribers.find(eventType) == m_Subscribers.end())
            {
                return;
            }

            for (const auto& subscriber : m_Subscribers[eventType])
            {
                subscriber(data);
            }
        }

        void Clear()
        {
            m_Subscribers.clear();
        }

    private:
        // Each subscriber is mapped to an event, which we use to retrieve and call said subscribers.
        std::unordered_map<EventType, std::vector<Subscriber>> m_Subscribers; 
    };
}