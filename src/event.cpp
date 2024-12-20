#include "event.h"

using namespace monokl;

Action::Action(ActionType type)
  : type(type) {}

Action::Action(ActionType type, const std::string& text_data)
  : type(type),
    text_data(text_data)
    {}

WindowEvent::WindowEvent(WindowEventType type)
  : type(type)
    {}

Event::Event(unsigned int window_id, std::shared_ptr<Action> action)
  : window_id(window_id),
    type(EventType::ActionEvent),
    action(action)
    {}

Event::Event(unsigned int window_id, std::shared_ptr<WindowEvent> event)
  : window_id(window_id),
    type(EventType::WindowEvent),
    window(event)
    {}

Event::~Event() {
  if (action != nullptr) {
    action.reset();
  }

  if (window != nullptr) {
    window.reset();
  }
}

EventBus::EventBus() {
}

EventBus::~EventBus() {
  callbacks.clear();
}

void EventBus::publish(std::shared_ptr<Event> event) {
  for (const auto& [id, callback] : callbacks) {
    callback(event);
  }

  for (const auto& id : pending_unsubscribes) {
    callbacks.erase(id);
  }
}

EventBus::CallbackId EventBus::subscribe(EventCallback callback) {
  CallbackId id = next_callback_id++;
  callbacks[id] = callback;
  return id;
}

void EventBus::unsubscribe(EventBus::CallbackId id) {
  pending_unsubscribes.push_back(id);
}
