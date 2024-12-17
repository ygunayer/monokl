#pragma once

#include <functional>
#include <map>

#include "action.h"

namespace monokl {

enum class EventType {
  ActionEvent,
  WindowEvent,
};

enum class WindowEventType {
  GainedFocus,
  LostFocus,
  Resized,
  Closed,
  Maximized,
  Minimized,
  Restored,
};

struct WindowEvent {
  WindowEventType type;

  explicit WindowEvent(WindowEventType type);
};

struct Event {
  unsigned int window_id = 0;
  EventType type;
  std::shared_ptr<Action> action;
  std::shared_ptr<WindowEvent> window;

  explicit Event(unsigned int window_id, const Action& action);
  explicit Event(unsigned int window_id, const WindowEvent& event);

  ~Event();
};

struct EventBus {
  using EventCallback = std::function<void(const Event&)>;
  using CallbackId = unsigned long;

  void publish(const Event& event);
  CallbackId subscribe(const EventCallback& callback);
  void unsubscribe(CallbackId callback_id);

  EventBus();
  ~EventBus();

private:
  CallbackId next_callback_id = 1l;
  std::unordered_map<int, EventCallback> callbacks;
};

};
