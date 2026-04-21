#pragma once

#include <functional>
#include <map>
#include <vector>
#include <string>
#include <any>
#include <mutex>

namespace Core {

// Deep comment: The EventBus replaces Win32 SendMessage for internal communication.
// Why: Win32 couples the application to Windows. An EventBus allows platform-agnostic
//      core logic to trigger events that Qt/GTK frontends can listen to.
// Side effects: Introduces slightly more heap allocation for event payloads compared
//               to passing raw pointers via SendMessage.
// Alternatives: Direct observer pattern (interfaces), but EventBus offers looser coupling.

class EventBus {
public:
    using EventCallback = std::function<void(const std::any&)>;
    using EventID = std::string;

    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    void Subscribe(const EventID& eventId, EventCallback callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_listeners[eventId].push_back(callback);
    }

    void Publish(const EventID& eventId, const std::any& payload = {}) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_listeners.find(eventId) != m_listeners.end()) {
            for (auto& callback : m_listeners[eventId]) {
                callback(payload);
            }
        }
    }

private:
    EventBus() = default;
    ~EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    std::map<EventID, std::vector<EventCallback>> m_listeners;
    std::mutex m_mutex;
};

} // namespace Core
