#include "event.h"

using namespace monokl;

Action::Action(ActionType type)
  : type(type) {}

Action::Action(ActionType type, const std::string& text_data)
  : type(type),
    text_data(text_data)
    {}

Event::Event(unsigned int window_id, const Action& action)
  : window_id(window_id),
    type(EventType::ActionEvent),
    action(action)
    {}

EventBus::EventBus() {
}

EventBus::~EventBus() {
  callbacks.clear();
}

void EventBus::publish(const Event& event) {
  for (const auto& [id, callback] : callbacks) {
    callback(event);
  }
}

EventBus::CallbackId EventBus::subscribe(const EventCallback& callback) {
  CallbackId id = next_callback_id++;
  callbacks[id] = callback;
}

void EventBus::unsubscribe(EventBus::CallbackId id) {
  callbacks.erase(id);
}
