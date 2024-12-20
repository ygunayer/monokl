#pragma once

#include <functional>
#include <map>
#include <memory>

#include "action.h"
#include "logging.h"

namespace monokl {

enum class EventType {
  ActionEvent,
  WindowEvent,
};

enum class WindowEventType {
  Created,
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

  explicit Event(unsigned int window_id, std::shared_ptr<Action> action);
  explicit Event(unsigned int window_id, std::shared_ptr<WindowEvent> event);

  ~Event();
};

struct EventBus {
  using EventCallback = std::function<void(std::shared_ptr<Event>)>;
  using CallbackId = unsigned long;

  void publish(std::shared_ptr<Event> event);
  CallbackId subscribe(EventCallback callback);
  void unsubscribe(CallbackId callback_id);

  EventBus();
  ~EventBus();

private:
  CallbackId next_callback_id = 1l;
  std::unordered_map<int, EventCallback> callbacks;

  std::vector<CallbackId> pending_unsubscribes;
};

};
