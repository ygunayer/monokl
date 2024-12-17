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

Event::Event(unsigned int window_id, const Action& action)
  : window_id(window_id),
    type(EventType::ActionEvent),
    action(std::make_shared<Action>(action))
    {}

Event::Event(unsigned int window_id, const WindowEvent& event)
  : window_id(window_id),
    type(EventType::WindowEvent),
    window(std::make_shared<WindowEvent>(event))
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

void EventBus::publish(const Event& event) {
  for (const auto& [id, callback] : callbacks) {
    callback(event);
  }
}

EventBus::CallbackId EventBus::subscribe(const EventCallback& callback) {
  CallbackId id = next_callback_id++;
  callbacks[id] = callback;
  return id;
}

void EventBus::unsubscribe(EventBus::CallbackId id) {
  callbacks.erase(id);
}
