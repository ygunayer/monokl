#pragma once

#include <functional>
#include <map>

namespace monokl {

enum class EventType {
  ActionEvent,
};

enum class ActionType {
  OpenNewWindow,
  CloseWindow,
  Quit,
  BeginDropFiles,
  DropFile,
  RefreshWindowSize,
  EndDropFiles,
  ToggleFavorite,
  ToggleFavoritesOnly,
  GoToNext,
  GoToPrevious,
  GoToFirst,
  GoToLast,
  ZoomIn,
  ZoomOut,
  FitImageToScreen,
  ResetZoom,
};

struct Action {
  ActionType type;
  std::string text_data;

  explicit Action(ActionType type);
  explicit Action(ActionType type, const std::string& text_data);
};

struct Event {
  unsigned int window_id = 0;
  EventType type;
  Action action;

  explicit Event(unsigned int window_id, const Action& action);
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
