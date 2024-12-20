#pragma once

#include <string>
#include <vector>
#include <optional>

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_scancode.h>

namespace monokl {

enum class ActionType {
  OpenNewWindow,
  CloseWindow,
  MaximizeWindow,
  MinimizeWindow,
  OpenFiles,
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

struct ActionMapping {
  SDL_KeyCode key;
  std::optional<uint16_t> flags;
  ActionType action;

  ActionMapping(SDL_KeyCode key, ActionType action);
  ActionMapping(SDL_KeyCode key, uint16_t flags, ActionType action);

  bool matches(const SDL_KeyboardEvent& event) const;
};

struct Action {
  ActionType type;
  std::string text_data;

  explicit Action(ActionType type);
  explicit Action(ActionType type, const std::string& text_data);
};

struct OpenFilesAction : public Action {
  std::vector<std::string> files;

  OpenFilesAction(const std::vector<std::string>& files);
};

};
